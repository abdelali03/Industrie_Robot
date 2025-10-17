//*******************************************************************
#include "EmbSysLib.h"
#include "Module/Rtos/Rtos.h"
#include "ReportHandler.h"
#include "config.h"
#include <stack>

class myTimerTask1 : public TaskManager::Task {
public:
    //---------------------------------------------------------------
    myTimerTask1(TaskManager &taskManager) {
        counter = 0;
        taskManager.add(this);
    }

    //---------------------------------------------------------------
    virtual void update(void) {
        counter++;  // Zählt jede 1ms auf
    }

    //---------------------------------------------------------------
    static DWORD counter;  // Statische Variable für den Zähler
};

DWORD myTimerTask1::counter = 0;


struct Position {
    int radius;
    int winkel;
    int hoehe;
    bool aufmachen;
    bool zumachen;

    Position() : radius(0), winkel(0), hoehe(0), aufmachen(false), zumachen(false) {}  // Standardkonstruktor
    Position(int r, int w, int h, bool auf,bool zu) : radius(r), winkel(w), hoehe(h), aufmachen(auf),zumachen(zu) {}
};

class Motor{

	public:

		Digital& plus;
		Digital& minus;
		Digital& inc;
		Digital& end;
		Timer_Mcu& timer;
		DWORD lastMoveTime = 0;

		int lastPosition = 0;
		int lastState = Digital::Event::NONE;
		int zielPosition;
		int isPosition=0;
		int currentState;
		int grenze;

		bool zielerreicht;
		bool aktiv = false;
		bool richtung;
		bool isBlockiert;
		enum class State { Idle, Running, Homing };
		State state = State::Idle;

		Motor(Digital& p, Digital& m, Digital& inc, Digital& e, Timer_Mcu& t, int grenze = 100)
		: plus(p), minus(m), inc(inc), end(e), timer(t), isPosition(0), grenze(grenze) {}

		void fahreZu(int ziel) {
	    	if (ziel < 0) ziel = 0;
	    	if (ziel > grenze) ziel = grenze;
	    	if (isPosition == ziel) {
	    	    zielerreicht = true;
	    	    aktiv = false;
	    	    return;
	    	}
	    	isBlockiert = false;
	    	zielPosition=ziel;
	    	richtung = (ziel >= isPosition);
	        aktiv = true;
	        zielerreicht=false;
	        lastMoveTime = myTimerTask1::counter;
	        lastPosition = isPosition;
	    }

	    void vorwaerts(){

	    	if (isPosition < zielPosition && isPosition < grenze) {
	    	    minus = 1;
	    	    plus = 0;

	    	} else {
	    		handleZielErreicht();
	    	}
	    }

	    void rueckwaerts(){

	    	if (isPosition > zielPosition) {
	    	    if (end == 1) {
	    	        isPosition = 0;
	    	        stop();
	    	        return;
	    	    }

	    	    plus = 1;
	    	    minus = 0;

	    	} else {
	    		handleZielErreicht();
	    	}


	    }
	    void home() {
	         state = State::Homing;
	         aktiv = true;
	         richtung = false;        // rückwärts

	     }

	    void update() {
	        // 1) Homing abarbeiten
	        if (state == State::Homing) {

	            // Motor rückwärts ansteuern
	            plus  = 1;
	            minus = 0;
	            // Endschalter prüfen
	            if (end == 1) {
	                stop();
	                isPosition = 0;
	                zielerreicht = true;
	                state = State::Idle;

	            }
	            return;
	        }

	        if (!aktiv) return;
	        //steuern
	        if (richtung) vorwaerts();
	        else rueckwaerts();
	        //inkrementierungsalgebra
	        currentState = inc.getEvent();
	        if (currentState != Digital::Event::NONE && currentState != lastState) {
	        	  if (richtung) isPosition++;
	        	  else isPosition--;
	        	  lastMoveTime = myTimerTask1::counter;
	        }
	        lastState = currentState;
	        //timer für ein blockiertes Motor
	        DWORD now =myTimerTask1::counter;
	        if ((now - lastMoveTime) > 1000 && lastPosition == isPosition) {
	        	isBlockiert=true;
	            stop();
	        }
	        lastPosition = isPosition;
	    }
	    void pause() {
	        if (aktiv) {
	            plus = 0;
	            minus = 0;
	        }
	        aktiv = !aktiv;
	    }
	    void stop() {
	   	        plus = 0;
	   	        minus = 0;
	   	        aktiv = false;
	   	    }

	    void reset() {
	     if (isPosition != 0) fahreZu(0);
	    }
	    void handleZielErreicht() {
	        zielerreicht = true;
	        stop();
	    }
};

//**********************************

class Robot{
public:

	Motor radius;
	Motor winkel;
	Motor hoehe;
	Motor greifer;

	int radiuspos;
	int winkelpos;
	int hoehepos;

	bool doesreset=false;
	bool startrobot=false;
	bool  einstellen=false;
	std::stack<Position> positionen;
	Position p;

	  enum class RobotState {

	      GreiferZu,          // Greifer schließen
	      WartenGreifer,    // warten bis Greifer zu
	      GreiferAuf,         // Greifer öffnen
	      Fertig              // Sequenz beendet
	  };
	  RobotState state {RobotState::Fertig};

	 Robot(Digital& r_p, Digital& r_m, Digital& r_i, Digital& r_e,
	          Digital& w_p, Digital& w_m, Digital& w_i, Digital& w_e,
	          Digital& h_p, Digital& h_m, Digital& h_i, Digital& h_e,
	          Digital& g_p, Digital& g_m, Digital& g_i, Digital& g_e,
	          Timer_Mcu& t)
	        : radius(r_p, r_m, r_i, r_e, t, 200),
	          winkel(w_p, w_m, w_i, w_e, t, 200),
	          hoehe(h_p, h_m, h_i, h_e, t, 200),
	          greifer(g_p, g_m, g_i, g_e, t, 100)
	    {}



    void goTO(int radius, int winkel, int hoehe){
    	this->winkel.fahreZu(winkel);
    	this->radius.fahreZu(radius);
    	this->hoehe.fahreZu(hoehe);

    }
    void start() {
        positionen = std::stack<Position>();


        positionen.push(Position(0, 0, 0,false,false));
        positionen.push(Position(100, 133, 95,true,false));
        positionen.push(Position(75, 80, 160,false,false));
        positionen.push(Position(75, 70, 160,false,true));
        positionen.push(Position(75, 70, 160,false,false));
        positionen.push(Position(10, 70, 160,false,false));
        positionen.push(Position(10, 10, 160,false,false));
        positionen.push(Position(10, 10, 10,false,false));
        positionen.push(Position(100, 137, 137,true,false));
        positionen.push(Position(75, 80, 60,false,false));
        positionen.push(Position(75, 72, 73,false,true));
        positionen.push(Position(75, 72, 73,false,false));
        positionen.push(Position(0, 72, 73,false,false));
        positionen.push(Position(0, 0, 0,false,false));


        einstellen     = true;
        startrobot     = true;
        state          = RobotState::Fertig;
        doesreset      = false;
        nichtBlockiert();
        radius.zielerreicht = false;
        winkel.zielerreicht = false;
        hoehe.zielerreicht = false;
        greifer.zielerreicht = false;
    }


   void GreiferSchliessen(){
    		greifer.fahreZu(75);
    		//30 ist maximal
   }
    void GreiferOeffnen(){
        		greifer.fahreZu(0);
        }

    void HomingRobot(){
    	 radius.home();
    	 winkel.home();
    	 hoehe.home();
    	 greifer.home();
    	 if (radius.zielerreicht && winkel.zielerreicht && hoehe.zielerreicht && greifer.zielerreicht)
    		 radius.zielerreicht = winkel.zielerreicht = hoehe.zielerreicht = greifer.zielerreicht = false;
    	 startrobot=false;
    }

    void update(){
    	if(startrobot){

            if (radius.zielerreicht && winkel.zielerreicht && hoehe.zielerreicht) {
                switch (state) {
                case RobotState::GreiferZu:   GreiferSchliessen(); state = RobotState::WartenGreifer; break;
                case RobotState::GreiferAuf:  GreiferOeffnen();    state = RobotState::WartenGreifer; break;
                case RobotState::WartenGreifer:
                	if(greifer.isBlockiert){greifer.zielerreicht = true;

                	greifer.isBlockiert = false;}
                	if (greifer.zielerreicht) { greifer.zielerreicht = false; state = RobotState::Fertig; }
                	break;
                case RobotState::Fertig:
                    radius.zielerreicht = winkel.zielerreicht = hoehe.zielerreicht = greifer.zielerreicht = false;
                    einstellen = true;
                    break;
                }
            }

            if (einstellen) {
                           if (!positionen.empty()) {
                               p = positionen.top();
                               positionen.pop();
                               goTO(p.radius, p.winkel, p.hoehe);
                               nichtBlockiert();
                               if      (p.zumachen)  state = RobotState::GreiferZu;
                               else if (p.aufmachen) state = RobotState::GreiferAuf;
                               else                      state = RobotState::Fertig;
                           } else {
                               startrobot = false;
                               stop();
                           }
                           einstellen = false;
                       }
    	}

    }
    void nichtBlockiert(){
    	this->radius.isBlockiert=false;
    	        this->winkel.isBlockiert=false;
    	      	this->hoehe.isBlockiert=false;
    	      	this->greifer.isBlockiert=false;
    }
    void motorenupdate(){
        this->radius.update();
        this->winkel.update();
      	this->hoehe.update();
      	this->greifer.update();
    }

    void pause(){
    	 this->radius.pause();
    	 this->winkel.pause();
    	 this->hoehe.pause();
    	 this->greifer.pause();
    }

     void stop(){
    	 radius.stop();
    	 winkel.stop();
    	 hoehe.stop();
    	 greifer.stop();
     }

     void reset(){
    	 radius.reset();
    	 winkel.reset();
    	 hoehe.reset();
    	 greifer.reset();
     	 }

 };



//*******************************************************************



class myTimerTask : public TaskManager::Task
{
    public:
    //---------------------------------------------------------------
    myTimerTask( TaskManager &taskManager, Robot &robot)
    : robot(robot) {
        cnt = 0;
        taskManager.add( this );
    }

    //---------------------------------------------------------------

    virtual void update( void )
    {
      cnt++;
      robot.update();
      robot.motorenupdate();
    }

    //---------------------------------------------------------------
    DWORD cnt;



    private:
    Robot &robot;
};

//*******************************************************************
class myRtosTask : public Rtos::Task
{
  public:
    //---------------------------------------------------------------
    myRtosTask( Rtos &rtos )
    : Rtos::Task( rtos, 500/* stack size*/ )
    {
      cnt = 0;
    }

  private:
    //---------------------------------------------------------------
    virtual void update( void )
    {
      while(1)
      {
        cnt++;
        pause();  // pause the task until next time slot
      }
    }

  public:
    //---------------------------------------------------------------
    DWORD cnt;

}; //class myTask

//*******************************************************************
Rtos    rtos (    2,   // max num of tasks
               1000 ); // time slice in us

//*******************************************************************
int main(void)
{
/*
  disp.printf(0,0,__DATE__ " " __TIME__);
  terminal.printf( __DATE__ " " __TIME__ "\r\n" );
*/
  int  num = 0;
  myRtosTask  rtosTask ( rtos );

  rtosTask.start();

    Robot robot(
        radius_plus, radius_minus, increment_radius, endschalter_radius,
        winkel_plus, winkel_minus, increment_winkel, endschalter_winkel,
        hoehe_plus, hoehe_minus, increment_hoehe, endschalter_hoehe,
        greifer_plus, greifer_minus, increment_greifer, endschalter_greifer,
        timer
    );

    myTimerTask timerTask(taskManager, robot);
    myTimerTask1 timerTask1(taskManagermotor);


  while(1)

  {
    if( char *str = terminal.getString() )
    {
//    terminal.printf( "\r\n=>%s\r\n", str );
    }

    switch( enc.getEvent() )
    {
        case DigitalEncoder::LEFT:     num -= 1; break;
        case DigitalEncoder::RIGHT:    num += 1; break;
        case DigitalEncoder::CTRL_DWN: num  = 0; break;
        default:                                 break;
    }

    if( btnA.getEvent() == Digital::ACTIVATED )
       {

    		robot.HomingRobot(); // man kann das auch als reset betrachten
           disp.printf(3, 0, "__Homing__");
       }
  	disp.printf( 0, 0, "hoehe%-5d", robot.hoehe.isPosition);
    disp.printf( 1, 0, "winkel%-5d", robot.winkel.isPosition);
    disp.printf( 2,0, "radius%-5d", robot.radius.isPosition);
    if( btnB.getEvent() == Digital::ACTIVATED )
           {
               led0.toggle();
               robot.start();
               disp.printf(3, 0, "__Start__");
           }
    if( btnC.getEvent() == Digital::ACTIVATED ){
    	robot.stop();
    	 disp.printf(3, 0, "__Stop__");
    }
    if( btnD.getEvent() == Digital::ACTIVATED ){
       robot.pause();
       disp.printf(3, 0, "__Pause__");
       }

    disp.refresh();
  }
}
//EOF
