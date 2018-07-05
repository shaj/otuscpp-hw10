#pragma once

#include <vector>
#include <list>
#include <istream>
#include <memory>


class Bulk
{
	std::vector<std::string> data;
	std::string m_id;
public:

	Bulk();

	auto begin()->decltype(data.begin());
	auto end()->decltype(data.end());
	const auto cbegin()->decltype(data.cbegin());
	const auto cend()->decltype(data.cend());

	void append(const std::string &s);
	std::string id();
	std::size_t size();
	std::string to_str();

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

	Bulk_Reader() = delete;

public:
	Bulk_Reader(std::istream &_is, std::size_t c);

	void add_printer(const std::weak_ptr<Bulk_Printer> &p);
	void remove_printer(const std::weak_ptr<Bulk_Printer> &p);

	void create_bulk();
	void append_bulk(const std::string &s);
	void close_bulk();
	// void push_bulk();

	void process();
	void notify(Bulk &b);
	
};


class Bulk_Printer
{
protected:
	std::weak_ptr<Bulk_Reader> reader;
public:
	virtual void update(Bulk &b) = 0;	
};


class Con_Printer: public Bulk_Printer
{
	Con_Printer() = delete;
	Con_Printer(const std::weak_ptr<Bulk_Reader> &r);
public:	
	static std::shared_ptr<Con_Printer> create(const std::weak_ptr<Bulk_Reader> &r);
	void update(Bulk &b) override;
};


class File_Printer: public Bulk_Printer
{
	File_Printer() = delete;
	File_Printer(const std::weak_ptr<Bulk_Reader> &r);
public:	
	static std::shared_ptr<File_Printer> create(const std::weak_ptr<Bulk_Reader> &r);
	void update(Bulk &b) override;
};
