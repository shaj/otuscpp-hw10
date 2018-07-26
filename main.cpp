


#include <iostream>
#include "log.h"
#include "version.h"
#include "metr.h"
#include "bulk.h"
#include "tp.h"


#include <boost/program_options.hpp>

namespace po = boost::program_options;


void set_bulk(std::size_t val)
{
	// std::cout << "bulk size is " << val << std::endl;
}

std::shared_ptr<spdlog::logger> my::my_logger;

int main (int argc, char* argv[])
{
	try
	{
		my::my_logger = spdlog::basic_logger_mt("mainlogger", "bulk.log", true);
		my::my_logger->set_level(spdlog::level::info);
		my::my_logger->info(" -=- Start bulk");

		po::options_description descr("Allowed options");
		descr.add_options()
			("help,h", "Produce help message")
			("version,v", "Version")
			("debug,d", "Enable tracing")
			("bulk,b", po::value<std::size_t>()->default_value(3)->notifier(set_bulk), "bulk size")
			("thread-pool,p", po::value<std::size_t>()->default_value(3), "thread pool size")
		;

		po::positional_options_description p;
		p.add("bulk", -1);
		po::variables_map vm;
		// po::store(po::parse_command_line(argc, argv, descr), vm);
		po::store(po::command_line_parser(argc, argv).options(descr).positional(p).run(), vm);
		po::notify(vm);

		if(vm.count("help"))
		{
			std::cout << descr << std::endl;
			return 0;
		}

		if(vm.count("version"))
		{
			std::cout << "OTUS cpp\n";
			std::cout << "Homework 10. Bulk mt.\n";
			std::cout << "Version " << VER << "\n" << std::endl;
			return 0;
		}

		if(vm.count("debug"))
		{
			my::my_logger->set_level(spdlog::level::trace);
			my::my_logger->flush_on(spdlog::level::trace); 
		}


		// my::my_logger->info("Thread pool size {}; bulk size {}", vm["thread-pool"].as<size_t>(), vm["bulk"].as<size_t>());

		auto m_main = std::make_shared<Metr>();
		auto tpool  = std::make_shared<ThreadPool>(vm["thread-pool"].as<size_t>());

		auto reader = std::make_shared<Bulk_Reader>(std::cin, vm["bulk"].as<size_t>(), tpool);
		auto console = Con_Printer::create(reader);
		auto file = File_Printer::create(reader);

		SPDLOG_TRACE(my::my_logger, "main   start process");

		reader->process(m_main);

		SPDLOG_TRACE(my::my_logger, "main   end process");

		// Завершаем пул потоков для того, чтобы получить полную статистику
		tpool->join();

		SPDLOG_TRACE(my::my_logger, "main   end join");

		std::cout << "\nmain поток - " << m_main->str_cnt << 
				" строк, " << m_main->cmd_cnt <<
				" команд, " << m_main->blk_cnt << " блоков\n" << std::endl;

		int ti = 0;
		Metr m_summ;
		for(const auto &it : tpool->get_metr())
		{
			std::cout << "Поток " << ti << " выполнен " << it->cnt << " раз. " << it->blk_cnt << " блоков. " << it->cmd_cnt << " команд.\n";
			m_summ.cnt += it->cnt;
			m_summ.str_cnt += it->str_cnt;
			m_summ.blk_cnt += it->blk_cnt;
			m_summ.cmd_cnt += it->cmd_cnt;
			ti++;
		}
		std::cout << "\nСумма по потокам: " << m_summ.cnt << " вызовов. " << m_summ.str_cnt << " прочитано строк. "
				<< m_summ.blk_cnt << " блоков. " << m_summ.cmd_cnt << " команд.\n";
		std::cout << std::endl;

		SPDLOG_TRACE(my::my_logger, "main   end report");

		my::my_logger->info(" -=- End bulk");

	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
