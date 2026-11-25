#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
struct State {virtual ~State()=default;virtual bool allows(const std::string&)const=0;};
struct NewState:State{bool allows(const std::string& s)const override{return s=="Verified"||s=="Rejected";}};
struct VerifiedState:State{bool allows(const std::string& s)const override{return s=="Processed"||s=="Rejected";}};
struct TerminalState:State{bool allows(const std::string&)const override{return false;}};
inline std::unique_ptr<State> stateFor(const std::string& s){if(s=="New")return std::make_unique<NewState>();if(s=="Verified")return std::make_unique<VerifiedState>();return std::make_unique<TerminalState>();}
struct Inputs{double importance,urgency,waiting;};
struct Strategy{virtual ~Strategy()=default;virtual double calculate(Inputs)const=0;};
struct Maximum:Strategy{double calculate(Inputs x)const override{return std::max({x.importance,x.urgency,x.waiting});}};
struct Weighted:Strategy{double calculate(Inputs x)const override{return .5*x.importance+.3*x.urgency+.2*x.waiting;}};
struct Normalized:Strategy{double calculate(Inputs x)const override{return .5*x.importance/10+.3*x.urgency/10+.2*x.waiting/60;}};
struct StrategyFactory{virtual ~StrategyFactory()=default;virtual std::unique_ptr<Strategy> create()const=0;};
template<class T> struct Factory:StrategyFactory{std::unique_ptr<Strategy> create()const override{return std::make_unique<T>();}};
class Priority{std::unique_ptr<Strategy> s;public:void set(const StrategyFactory& f){s=f.create();}double calculate(Inputs x)const{if(!s)throw std::logic_error("No strategy");return s->calculate(x);}};
struct Event{int id;std::string type;};
struct Observer{virtual ~Observer()=default;virtual void update(const Event&)=0;};
struct Audit:Observer{int count=0;void update(const Event& e)override{++count;std::cout<<"Audit: "<<e.type<<" #"<<e.id<<'\n';}};
struct Statistics:Observer{int received=0,processed=0;void update(const Event& e)override{if(e.type=="Received")++received;if(e.type=="Processed")++processed;}};
class Publisher{std::vector<std::weak_ptr<Observer>> subs;public:void subscribe(const std::shared_ptr<Observer>& p){subs.push_back(p);}void unsubscribe(const std::shared_ptr<Observer>& p){std::erase_if(subs,[&](auto& w){return w.expired()||w.lock()==p;});}void notify(const Event& e){for(auto& w:subs)if(auto p=w.lock())p->update(e);}};
