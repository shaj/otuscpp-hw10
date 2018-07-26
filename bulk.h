#pragma once

#include <vector>
#include <list>
#include <istream>
#include <memory>
#include <mutex>

#include "metr.h"
#include "tp.h"


class Bulk
{
	std::vector<std::string> data;   ///< Хранилище команд
	std::string m_id;                ///< Идентификатор булька
public:

	Bulk();

	auto begin()->decltype(data.begin());
	auto end()->decltype(data.end());
	const auto cbegin()->decltype(data.cbegin());
	const auto cend()->decltype(data.cend());

	void append(const std::string &s);
	std::string id() const;
	std::size_t size() const;
	std::string to_str() const;

private:
	void update_id();	
};


class Bulk_Printer;


class Bulk_Reader
{
	std::list<Bulk> bulks;
	std::list<std::weak_ptr<Bulk_Printer>> printers;
	std::istream &is;
	std::size_t bulk_size;
	std::size_t bulk_cnt;
	int level;

	std::shared_ptr<Metr> metr;
	std::shared_ptr<ThreadPool> tpool;

	Bulk_Reader() = delete;

public:
	Bulk_Reader(std::istream &_is, std::size_t c, std::shared_ptr<ThreadPool> tp);

	void add_printer(const std::weak_ptr<Bulk_Printer> &p);
	void remove_printer(const std::weak_ptr<Bulk_Printer> &p);

	void process(std::shared_ptr<Metr> m = nullptr);

private:	
	void create_bulk();
	void append_bulk(const std::string &s);
	void close_bulk();

	void notify(const Bulk &b);
	
};


class Bulk_Printer
{
protected:
	std::weak_ptr<Bulk_Reader> reader;
public:
	virtual void print(const Bulk &b, std::shared_ptr<Metr> metr = nullptr) = 0;	
};


class Con_Printer: public Bulk_Printer
{
	static std::mutex con_m;

	Con_Printer() = delete;
	Con_Printer(const std::weak_ptr<Bulk_Reader> &r);
public:	
	static std::shared_ptr<Con_Printer> create(const std::weak_ptr<Bulk_Reader> &r);
	void print(const Bulk &b, std::shared_ptr<Metr> metr = nullptr) override;
};


class File_Printer: public Bulk_Printer
{
	static std::mutex fs_m;

	File_Printer() = delete;
	File_Printer(const std::weak_ptr<Bulk_Reader> &r);
public:	
	static std::shared_ptr<File_Printer> create(const std::weak_ptr<Bulk_Reader> &r);
	void print(const Bulk &b, std::shared_ptr<Metr> metr = nullptr) override;
};
