#ifndef FSM_H
#define FSM_H


#include <vector>
#include <algorithm>
#include <tuple>
#include <list>
#include <set>

#define ACTION []()
typedef void (*action)();

struct State{
  action onEnter;
  action onExit;
  bool _any;
  bool _stop;
  State( bool any=false, bool stop=false ):onEnter(0),onExit(0),_any(any), _stop(stop){}
};

class FSM{




 struct FsmRow{
   State &from; State &to; int at; action onTransition;
   FsmRow(State &from_, State &to_, int at, action a=0):from(from_),to(to_),at(at),onTransition(a){}
 };

 std::vector< FsmRow > _transitionTable;
 State *_start;
 State *_active;
 int _currentEvent;
 bool _run;

public:

 FSM():_start(0),_active(0){}

 bool setStartState( State & s ){
   _start = &s;
 }

  bool processEvent( int e )
  {
    _currentEvent = e;
    if ( !_active )
      return false;
    if ( !_run )
      return false;

    for ( FsmRow &r :_transitionTable )//TODO consider storing transitions in states
    {     
      if ( ( _active == &(r.from) || r.from._any ) && e == (r.at) )
      {

        if( _active->onExit )
          _active->onExit();

        if( r.onTransition )
          r.onTransition();

        if( r.to.onEnter  )
          r.to.onEnter();

        _active = &r.to;
        return true;
      }
    }
    return false;
  }

  bool start( )
  {
    if ( _start && !_run )
    {
      _active = _start;
      if( _active->onEnter )
      {       
        _active->onEnter();        
      }
       _run=true;
       return true;
    }
    return false;
  }

  void reset( ){ _run=false; _active = 0; }
  void pause( ){ _run=false; }
  void resume( ){ _run=true; processEvent( _currentEvent ); }

  // 1 transitions should be unique
  // 2 transitions should be unambigious
  // 3 not STOP-states should have to-transitions
  std::tuple<bool, std::string> check() const
  {
    std::tuple<bool, std::string> ret(true,"");
    std::list< std::tuple< State*, int > > transitions;
    for ( const FsmRow &r :_transitionTable )//TODO consider storing transitions in states
    {
      transitions.push_back( std::make_tuple( &(r.from), (r.at) ) );
    }

    std::set< std::tuple< State*, int > > tmpUniqueCheck;
    for ( std::tuple< State*, int > &r :transitions )
      tmpUniqueCheck.insert(r);

    if ( tmpUniqueCheck.size() !=  transitions.size() )
    {
      std::get<0>(ret) = false;
      std::get<1>(ret) = "Not unique combination of from-state, event";
    }


    ///
    std::set<State*> toStates;
    std::set<State*> fromStates;

    for ( FsmRow r: _transitionTable )
    {
      if ( !r.to._stop )
      {
        toStates.insert( &r.to );
        fromStates.insert(&r.from);
      }
    }
    std::list<State*> blindStates;

    std::set_difference(
          toStates.begin(), toStates.end(),
          fromStates.begin(), fromStates.end(),
          std::inserter(blindStates, blindStates.begin()) );
    if ( blindStates.size() !=  0 )
    {
      std::get<0>(ret) = false;
      std::get<1>(ret) = "Not all states have transition to another state";
    }
    ///

    return ret;
  }

  FSM & operator()( State &from, const char*, State &to, const char*, int at, action a=0 )
  {
    if ( _transitionTable.size() == 0 )
    {
      setStartState(from);
    }
    _transitionTable.push_back( FsmRow( from, to, at, a ) );
    return *this;
  }
};

#endif // FSM_H
