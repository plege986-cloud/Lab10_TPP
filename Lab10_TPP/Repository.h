#pragma once
#include <stdexcept>
#include "Database.h"
#include <optional>
#include <vector>
class IEventSourceRepository {
public:
 virtual ~IEventSourceRepository()=default;
 virtual int add(const EventSource&)=0;
 virtual std::optional<EventSource> find(int)=0;
 virtual std::vector<EventSource> all()=0;
 virtual bool update(const EventSource&)=0;
 virtual bool remove(int)=0;
};
class EventSourceRepository final: public IEventSourceRepository {
 Storage& db;
public:
 explicit EventSourceRepository(Storage& s):db(s){}
 int add(const EventSource& e) override {return db.insert(e);}
 std::optional<EventSource> find(int id) override {return db.get_optional<EventSource>(id);}
 std::vector<EventSource> all() override {return db.get_all<EventSource>();}
 bool update(const EventSource& e) override {if(!find(e.id))return false;db.update(e);return true;}
 bool remove(int id) override {if(!find(id))return false;db.remove<EventSource>(id);return true;}
};
class LogEntryStore {
 Storage& db;
public:
 explicit LogEntryStore(Storage& s):db(s){}
 int add(const LogEntry& e){return db.insert(e);}
 std::vector<LogEntry> all(){return db.get_all<LogEntry>();}
};
class StateRepository {
 Storage& db;
public:
 explicit StateRepository(Storage& s):db(s){}
 int create(LogEntry e){int id=0;db.transaction([&]{id=db.insert(e);db.insert(EntryState{0,id,"New"});return true;});return id;}
 EntryState get(int id){using namespace sqlite_orm;auto rows=db.get_all<EntryState>(where(c(&EntryState::entryId)==id));if(rows.empty())throw std::runtime_error("Missing state");return rows.front();}
 void change(int id,const std::string& next,bool fail=false){db.transaction([&]{auto e=get(id);auto old=e.status;e.status=next;db.update(e);db.insert(StateHistory{0,id,old,next});return !fail;});}
 int historyCount(){return db.count<StateHistory>();}
};
