
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <mutex>
// #include <cstdio>

#include "bulk.h"
#include "log.h"
#include "tp.h"


Bulk::Bulk()
{
	SPDLOG_TRACE(my::my_logger, "Bulk::Bulk()");

	update_id();
}

auto Bulk::begin()->decltype(data.begin())
{
	SPDLOG_TRACE(my::my_logger, "auto Bulk::begin()");

	return data.begin();
}

auto Bulk::end()->decltype(data.end())
{
	SPDLOG_TRACE(my::my_logger, "auto Bulk::end()");

	return data.end();
}

const auto Bulk::cbegin()->decltype(data.cbegin())
{
	SPDLOG_TRACE(my::my_logger, "auto Bulk::cbegin()");

	return data.cbegin();
}

const auto Bulk::cend()->decltype(data.cend())
{
	SPDLOG_TRACE(my::my_logger, "auto Bulk::cend()");

	return data.cend();
}

void Bulk::append(const std::string &s)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk::append");

	if(data.size() == 0) update_id();
	data.push_back(s);
}

void Bulk::update_id()
{
	std::time_t t = std::time(nullptr);
	m_id = std::to_string((long long)t);
}

std::string Bulk::id() const
{
	SPDLOG_TRACE(my::my_logger, "std::string Bulk::id");

	return m_id;
}

std::size_t Bulk::size() const
{
	SPDLOG_TRACE(my::my_logger, "std::size_t Bulk::size");

	return data.size();
}

std::string Bulk::to_str() const
{
	SPDLOG_TRACE(my::my_logger, "std::string Bulk::to_str");

	std::ostringstream retval;
	if(data.size() != 0)
	{
		auto it = data.cbegin();
		retval << *it;
		it++;
		for(; it != data.cend(); it++)
		{
			retval << ", " << *it;
		}
	}
	return retval.str();
}


Bulk_Reader::Bulk_Reader(std::istream &_is, std::size_t c, std::shared_ptr<ThreadPool> tp) : 
		is(_is), 
		bulk_size(c),
		level(0),
		tpool(tp)
{
	SPDLOG_TRACE(my::my_logger, "Bulk_Reader::Bulk_Reader");

	if(bulk_size < 1)
	{
		my::my_logger->warn("Bulk size is 0");
		std::cout << "Bulk size is 0" << std::endl;
	}

	SPDLOG_TRACE(my::my_logger, "   bulk_size={}", bulk_size);
}

void Bulk_Reader::add_printer(const std::weak_ptr<Bulk_Printer> &p)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::add_printer");

	printers.push_back(p);
}

void Bulk_Reader::remove_printer(const std::weak_ptr<Bulk_Printer> &prt)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::remove_printer");

	// Отсюда https://stackoverflow.com/a/10120851
	printers.remove_if([prt](const auto &p)
		{ 
			return !(p.owner_before(prt) || prt.owner_before(p));
		});

	// Про удаление из деструктора
	// https://stackoverflow.com/q/28338978
}

void Bulk_Reader::process(std::shared_ptr<Metr> m)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::process()");
	metr = m;
	if(is)
	{
		std::string str;
		create_bulk();
		while(std::getline(is, str))
		{
			if(metr) metr->str_cnt++;
			append_bulk(str);
		}
		close_bulk();
	}
}


void Bulk_Reader::create_bulk()
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::create_bulk()");

	bulks.emplace_back();
	bulk_cnt = bulk_size;
}

void Bulk_Reader::append_bulk(const std::string &s)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::append_bulk");

	if(level == 0)
	{
		if(s == "{")
		{
			if(bulk_cnt != bulk_size)
			{
				notify(*(--bulks.end()));
				create_bulk();
			}
			level++;
		}
		else if (s == "}")
		{
		}
		else if(bulk_cnt)
		{
			if(metr) metr->cmd_cnt++;
			(--bulks.end())->append(s);
			bulk_cnt--;
			if(bulk_cnt == 0)
			{
				notify(*(--bulks.end()));
				create_bulk();
			}
		}
		else
		{ // Сюда попадаем, если bulk_size == 0.
			my::my_logger->warn("Bulk size is 0");
			// do nothing
		}
	}
	else
	{
		if(s == "{") level++;
		else if(s == "}")
		{
			if(--level == 0)
			{
				notify(*(--bulks.end()));
				create_bulk();
			}
		}
		else
		{
			if(metr) metr->cmd_cnt++;
			(--bulks.end())->append(s);
		}
	}
}

void Bulk_Reader::close_bulk()
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::close_bulk()");

	if((level == 0) && (bulk_cnt != bulk_size))
		notify(*(--bulks.end()));
}

void Bulk_Reader::notify(const Bulk &b)
{
	SPDLOG_TRACE(my::my_logger, "void Bulk_Reader::notify");

	if(b.size() != 0)
	{
		if(metr) metr->blk_cnt++;
		for(const auto &it: printers)
		{
			if(auto prt = it.lock())
				if(tpool)
				{
					// tpool->msgs_put(&prt->print, std::ref(b));
					tpool->msgs_put([&](auto al, auto &bl, auto cl)
						{ al->print(bl, cl);}, prt, std::ref(b));
				}
				else
				{
					prt->print(b);
				}
			else my::my_logger->warn("Bulk_Printer expired now");
		}
	}
}



std::mutex Con_Printer::con_m;

Con_Printer::Con_Printer(const std::weak_ptr<Bulk_Reader> &r)
{
	SPDLOG_TRACE(my::my_logger, "Con_Printer::Con_Printer");
	reader = r;
}

void Con_Printer::print(const Bulk &b, std::shared_ptr<Metr> metr) 
{
	SPDLOG_TRACE(my::my_logger, "void Con_Printer::update");

	std::string str;
	if(b.size() != 0)
	{
		str = b.to_str();
		std::lock_guard<std::mutex> lk_con(con_m);
		std::cout << "bulk: " << str << "\n" << std::flush;
	}
	if(metr)
	{
		metr->blk_cnt++;
		metr->cmd_cnt += b.size();
	}
}

std::shared_ptr<Con_Printer> Con_Printer::create(const std::weak_ptr<Bulk_Reader> &r)
{
	SPDLOG_TRACE(my::my_logger, "static Con_Printer::create");

	auto ret = std::shared_ptr<Con_Printer>(new Con_Printer(r));
	r.lock()->add_printer(ret);
	return ret;
}





std::mutex File_Printer::fs_m;

File_Printer::File_Printer(const std::weak_ptr<Bulk_Reader> &r)
{
	SPDLOG_TRACE(my::my_logger, "File_Printer::File_Printer");
	reader = r;
}

void File_Printer::print(const Bulk &b, std::shared_ptr<Metr> metr) 
{
	SPDLOG_TRACE(my::my_logger, "void File_Printer::update");

	if(b.size() != 0)
	{
		std::string fname {"bulk" + b.id()};
		std::fstream fs;

		std::unique_lock<std::mutex> lk_fs(fs_m);

		fs.open(fname + ".log", std::ios::in);
		if(fs)
		{
			int cnt = 1;
			std::string fname_new;
			while(fs)
			{
				fs.close();
				fname_new = fname + "_" + std::to_string(cnt);
				fs.open(fname_new + ".log");
				cnt++;
				if(cnt > 100000) throw std::logic_error("File_Printer can not create log file");
			}
			fname = fname_new;
		}
		fs.close();
		
		fname += ".log";
		SPDLOG_TRACE(my::my_logger, "   fname={}", fname);

		std::fstream fso;
		try
		{ // Отсюда: https://stackoverflow.com/a/40057555
			fso.exceptions(std::fstream::failbit | std::fstream::badbit);
			fso.open(fname, std::ios::out);
			lk_fs.unlock();
			fso << "bulk: " << b.to_str();
			fso.close();
		}
		catch (std::fstream::failure &e) 
		{
			lk_fs.unlock();
			if(fso.is_open())
			{
				fso.close();
				std::remove(fname.c_str());
			}
			my::my_logger->error("File_Printer can not write data to file");
			throw std::runtime_error("File_Printer can not write data to file");
		}
		
		if(metr)
		{
			metr->blk_cnt++;
			metr->cmd_cnt += b.size();
		}
	}
}

std::shared_ptr<File_Printer> File_Printer::create(const std::weak_ptr<Bulk_Reader> &r)
{
	SPDLOG_TRACE(my::my_logger, "static File_Printer::create");

	auto ret = std::shared_ptr<File_Printer>(new File_Printer(r));
	r.lock()->add_printer(ret);
	return ret;
}


