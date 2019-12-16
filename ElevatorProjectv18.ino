/*
  Program Requirements:
  • All lab projects must include at least one usage of all of the following elements:
  • variable declaration
  • Serial communication
  • analog / digital I/O
  • structured command
  • loop (while, do while, or for)
  • array
  • data structure
  • user-defined function
  • interrupts

  5) Elevator Controller
  Design a controller for an elevator, with the following specifications:
  • 3 floor elevator
  • elevator can be called to any floor, by a button on each floor (3 button digital input)
  • elevator can be sent to floors by 3 buttons in the elevator (3 button digital input)
  • sensors (button) on each floor indicate if elevator has arrived, or is passing by (limit switches?)
  • indicators on each floor and in the elevator of present elevator location (3 LEDs x 2)
  • ordering and prioritization of floor calls (always follow chronological order of calls) (goes 1,2,3 then 3,2,1 order always passing each LED)
  • emergency stop (stops elevator immediately in place) - Use analog pin leftover for this we can make it digital input
  • emergency personnel elevator override (stops elevator, forces to main floor)
  • log path of travel, call times, and arrival times for each day
  • all logged values output serially at end of day
  • Fully commented program

   Created by
   Bryn Flewelling 100142811
*/

//FEATURE COMPLETED AFTER PRESENTATION WAS DATA LOGGER FUNCTIONALITY WITH STRUCT AND ARRAY - 150% FUNCTIONING NOW

//DEFINE CONSTANT MSTOP  ESTOP(GO TO FLOOR ONE)
const int MSTOP_Button = 2;

//DEFINE CONSTANT E STOP
const int ESTOP_Button = 3;

//DEFINE CONSTANT MSTOP ESTOP LED
//PIN 4 UNUSED
const int MSTOP_LED =  5;
const int ESTOP_LED =  6;

//DEFINE CONSTANTS FOR ELEVATOR REQUEST FLOOR BUTTONS
const int Floor_1_ELEVATOR_Button = 7;
const int Floor_2_ELEVATOR_Button = 8;
const int Floor_3_ELEVATOR_Button = 9;

//DEFINE ELEVATOR AT FLOOR DESTINATION FLOOR LEDS
const int FLOOR_1_LED =  10; //request floor1
const int FLOOR_2_LED =  11; //request floor2
const int FLOOR_3_LED =  12; //request floor3

//13 pin unused

//DEFINE CONSTANTS FOR FLOOR ELEVATOR REQUEST BOTTONS
const int Floor_3_Button = 14; //call floor3
const int Floor_2_Button = 15; //call floor2
const int Floor_1_Button = 16; //call floor1

//DIP SWITCH FLOOR LATCHES
const int Switch_3 = 17; //FLOOR LATCH F3
const int Switch_2 = 18; //FLOOR LATCH F2
const int Switch_1 = 19; //FLOOR LATCH F1

//SETUP BUTTON STATES FOR FLOORS AND SWITCHES
int Floor_1_ELEVATOR_ButtonState = 0;
int Floor_2_ELEVATOR_ButtonState = 0;
int Floor_3_ELEVATOR_ButtonState = 0;
int Floor_1_ButtonState = 0;
int Floor_2_ButtonState = 0;
int Floor_3_ButtonState = 0;
int Switch_1State = 0;
int Switch_2State = 0;
int Switch_3State = 0;
int MSTOP_ButtonState = 0;
int ESTOP_ButtonState = 0;

//track up to 7 Days for change logs to an array when wipe all logs
//we will reset after 48 Hours or severe power loss (no EEPROM write)

//Declaration of Floor Call Tracker Arraw (MAX 3 Calls)
int FloorCallTracker[3] = {0, 0, 0};

//Devlaration of Floor State Index (1, 2, 3)
int index_floor_state = 0;

//Declaration of LogInfo data logger index state (150 MAX)
int index_state = 0;

//UpTimeTracker tracking declaration
long Days = 0;
long Hours = 0;
long Minutes = 0;
long Seconds = 0;
String DateTime = "";
int floorVisit = 0;

//track up to 150 floor change logs to an array
//track up to 150 time logs for floor change logs to an structured array
struct LogInfo {
  String DateTime;
  int floorVisit;
} ;

//DEFINE ARRAY OF DATA STRUCTURE 150 MAX (RESET AFTER 7 DAYS)
struct LogInfo Data [150];

//STATUS TRACKING VARIABLES
int incoming = 0;
long currentmillis = 0;

/*****SetupFunction*****************************************
  Setup function
  Defines the pin types of inputs and outputs
  Defines Interupt pins
  Sets any power up conditions
  Prints Welcome Message on boot
*************************************************************/
void setup() {

  //DEFINE INPUTS AND OUTPUTS FOR ELEVATOR CONTROLLER
  pinMode(MSTOP_Button, INPUT);
  pinMode(ESTOP_Button, INPUT);

  pinMode(Floor_1_ELEVATOR_Button, INPUT);
  pinMode(Floor_2_ELEVATOR_Button, INPUT);
  pinMode(Floor_3_ELEVATOR_Button, INPUT);

  pinMode(Floor_1_Button, INPUT);
  pinMode(Floor_2_Button, INPUT);
  pinMode(Floor_3_Button, INPUT);

  pinMode(Switch_1, INPUT_PULLUP);
  pinMode(Switch_2, INPUT_PULLUP);
  pinMode(Switch_3, INPUT_PULLUP);

  //INTERRUPTS triggered by a change in state of the buttons D2, D3
  attachInterrupt (0, masterStop, CHANGE);
  attachInterrupt (1, emergencyStop, CHANGE);

  pinMode(FLOOR_1_LED, OUTPUT);
  pinMode(FLOOR_2_LED, OUTPUT);
  pinMode(FLOOR_3_LED, OUTPUT);

  pinMode(MSTOP_LED, OUTPUT);
  pinMode(ESTOP_LED, OUTPUT);

  //Start at floor one when power on
  digitalWrite(FLOOR_1_LED, HIGH);
  digitalWrite(FLOOR_2_LED, LOW);
  digitalWrite(FLOOR_3_LED, LOW);

  //SERIAL MONITOR
  Serial.begin(9600);

  //Prints the Welcome to Elevator Controller Message
  Welcome();
}

/*****LoopFunction*****************************************
  Loop function
  Executes the main code repeadly
  Contiunously monitors input states to execute
  function calls
*************************************************************/
void loop() {

  //READ BUTTON STATES FOR ELEVATOR REQUESTS
  Floor_1_ELEVATOR_ButtonState = digitalRead(Floor_1_ELEVATOR_Button);
  Floor_2_ELEVATOR_ButtonState = digitalRead(Floor_2_ELEVATOR_Button);
  Floor_3_ELEVATOR_ButtonState = digitalRead(Floor_3_ELEVATOR_Button);

  //READ BUTTON STATES FOR FLOOR REQUESTS
  Floor_1_ButtonState = digitalRead(Floor_1_Button);
  Floor_2_ButtonState = digitalRead(Floor_2_Button);
  Floor_3_ButtonState = digitalRead(Floor_3_Button);

  //READ ELEVATOR LATCH STATES FOR FLOOR LOCATION (SENSOR)
  Switch_1State = digitalRead(Switch_1);
  Switch_2State = digitalRead(Switch_2);
  Switch_3State = digitalRead(Switch_3);

  //READ EMERGENCY ESTOP AND MSTOP STATE
  MSTOP_ButtonState = digitalRead(MSTOP_Button);
  ESTOP_ButtonState = digitalRead(ESTOP_Button);

  //UpTimeTracker TRACKER FOR DATA LOGGER
  currentmillis = millis(); // get the current milliseconds
  UpTimeTracker(); //call conversion function to display date/time

  //BUG FIX: IF FloorCallTracker array 0 == 0 at this point reset index floor state
  //this will avoid any bugs occuring while running the program
  //INDEX FLOOR STATE SHOULD NEVER BE GREATER THAN ZERO IF FLOOR TRACKER IS ZERO
  if (FloorCallTracker[0] == 0) {
    index_floor_state = 0;
  }

  //ELEVATOR LATCH STATES LOGIC SETUP FOR FLOOR LOCATION (SENSOR) AND BUTTON PUSH
  //LOGS BUTTON PUSHES TO A ARRAY TO TRACK 3 MAX FLOOR CALLS AT A TIME
  if (Switch_1State == HIGH || Switch_2State == HIGH || Switch_3State == HIGH) {
    if (FloorCallTracker[0] == 0 || index_floor_state != 3) {
      if (Floor_1_ELEVATOR_ButtonState == HIGH || Floor_1_ButtonState == HIGH) {
        if (FloorCallTracker[0] != 1) {
          FloorCallTracker[index_floor_state] = 1;
          index_floor_state = (index_floor_state + 1) % 4;
          Serial.println(F("E CALL F 1 "));
        }
      } else if (Floor_2_ELEVATOR_ButtonState == HIGH || Floor_2_ButtonState == HIGH) {
        if (FloorCallTracker[0] != 2) {
          FloorCallTracker[index_floor_state] = 2;
          index_floor_state = (index_floor_state + 1) % 4;
          Serial.println(F("E CALL F 2 "));
        }
      } else if (Floor_3_ELEVATOR_ButtonState == HIGH || Floor_3_ButtonState == HIGH) {
        if (FloorCallTracker[0] != 3) {
          FloorCallTracker[index_floor_state] = 3;
          index_floor_state = (index_floor_state + 1) % 4;
          Serial.println(F("E CALL F 3 "));
        }
      }
      delay(1500);   //waiting for button presser
    }
  }

  //ELEVATOR LATCH STATES LOGIC FOR FLOOR LOCATION (SENSOR) AND TRIGGER
  //FUNCTION CALLS AT APPROPIATE MET LOGIC STATES
  if (Switch_1State == HIGH && Switch_2State == LOW && Switch_3State == LOW) {
    Floor1Function(FloorCallTracker);
  } else if (Switch_2State == HIGH && Switch_1State == LOW && Switch_3State == LOW) {
    Floor2Function(FloorCallTracker);
  } else if (Switch_3State == HIGH && Switch_2State == LOW && Switch_1State == LOW) {
    Floor3Function(FloorCallTracker);
  }

  //MASTER ESTOP PRESSED TRIGGER MSTOP FUNCTION
  if (MSTOP_ButtonState == HIGH) {
    masterStop();
  } else if (ESTOP_ButtonState == HIGH) { //ESTOP PRESSED TRIGGER ESTOP FUNCTION
    emergencyStop();
  } else if (Floor_3_ELEVATOR_ButtonState == HIGH && Floor_1_ELEVATOR_ButtonState == HIGH && Switch_1State == LOW && Switch_2State == LOW && Switch_3State == LOW) {
    PrintLogs();
  } else {
    //any error conditions that come up
  }
}

/*****Floor1 Function*****************************************
  Floor1 Function
  Accounts for all conditions while on the first floor
*************************************************************/
void Floor1Function(int FloorCallTracker[]) {
  //STATE tracking data
  Serial.println(F("index_floor_state"));
  Serial.println(index_floor_state);
  Serial.println(FloorCallTracker[0]);
  Serial.println(FloorCallTracker[1]);
  Serial.println(FloorCallTracker[2]);
  //FLOOR Set Status confirmation
  digitalWrite(FLOOR_1_LED, HIGH);
  digitalWrite(FLOOR_2_LED, LOW);
  digitalWrite(FLOOR_3_LED, LOW);

  //CHECK array State or State of FLOOR Call
  if (FloorCallTracker[0] == 2 || FloorCallTracker[0] == 1) {
    //WHEN switch toggle low, execute elevator move
    if (digitalRead(Switch_1) == LOW) {
      digitalWrite(FLOOR_3_LED, LOW);
      digitalWrite(FLOOR_1_LED, LOW);
      //UNTIL Elevator gets to Floor 2 LOOP
      while (digitalRead(Switch_2) != HIGH) {
        delay(1500);
      }
      //When Loop Breaks (AT FLOOR) EXECUTE FLOOR STATE
      if (digitalRead(Switch_2) == HIGH) {
        digitalWrite(FLOOR_2_LED, HIGH);
        Serial.println(F("FLOOR 2 REACHED"));
        //LOG FLOOR CALL DATA
        Data[index_state].DateTime = UpTimeTracker();
        Data[index_state].floorVisit = FloorCallTracker[0];
        index_state = (index_state + 1)  % 150;
        //SHIFT FLOOR CALL REGISTERES LEFT FOR NEXT FLOOR CALL STATE
        if (index_floor_state != 0) {
          FloorCallTracker[0] = FloorCallTracker[1];
          FloorCallTracker[1] = FloorCallTracker[2];
        }
        //BUGFIX CANT be FLOOR 2 AGAIN
        if (FloorCallTracker[0] == 2) {
          FloorCallTracker[0] = 0 ;
        }
        //TRACKING DATA FOR CONFIRMED FLOOR CALL SHIFT REGITER
        Serial.println(F("NEXT FLOOR SHIFT REGISTER"));
        Serial.println(FloorCallTracker[0]);
      }
      //delay timer for passengers to board elevator before next floor call
      delay(1500);
    }
  }
} //end function 1

/*****Floor2 Function*****************************************
  Floor2 Function
  Accounts for all conditions while on the second floor
*************************************************************/
void Floor2Function(int FloorCallTracker[]) {
  //STATE tracking data
  Serial.println(F("index_floor_state"));
  Serial.println(index_floor_state);
  Serial.println(FloorCallTracker[0]);
  Serial.println(FloorCallTracker[1]);
  Serial.println(FloorCallTracker[2]);
  //FLOOR Set Status confirmation
  digitalWrite(FLOOR_1_LED, LOW);
  digitalWrite(FLOOR_2_LED, HIGH);
  digitalWrite(FLOOR_3_LED, LOW);

  //CHECK array State or State of FLOOR Call IF CALL FLOOR 1
  if (FloorCallTracker[0] == 1) {
    //WHEN switch toggle low, execute elevator move
    if (digitalRead(Switch_2) == LOW) {
      digitalWrite(FLOOR_3_LED, LOW);
      digitalWrite(FLOOR_2_LED, LOW);
      //UNTIL Elevator gets to Floor 2 LOOP
      while (digitalRead(Switch_1) != HIGH) {
        delay(1500);
      }
      //When Loop Breaks (AT FLOOR) EXECUTE FLOOR STATE
      if (digitalRead(Switch_1) == HIGH) {
        digitalWrite(FLOOR_1_LED, HIGH);
        Serial.println(F("FLOOR 1 REACHED"));
        //BUGFIX CANT be FLOOR 2 AGAIN
        if (FloorCallTracker[0] == 2) {
          FloorCallTracker[0] = 0 ;
        }
        //LOG FLOOR CALL DATA
        Data[index_state].DateTime = UpTimeTracker();
        Data[index_state].floorVisit = FloorCallTracker[0];
        index_state = (index_state + 1)  % 150;
        //SHIFT FLOOR CALL REGISTERES LEFT FOR NEXT FLOOR CALL STATE
        if (index_floor_state != 0) {
          FloorCallTracker[0] = FloorCallTracker[1];
          FloorCallTracker[1] = FloorCallTracker[2];
        }
        //TRACKING DATA FOR CONFIRMED FLOOR CALL SHIFT REGITER
        Serial.println(F("NEXT FLOOR SHIFT REGISTER"));
        Serial.println(FloorCallTracker[0]);
      }
      //delay timer for passengers to board elevator before next floor call
      delay(1500);
    }
  } else if (FloorCallTracker[0] == 3) { //CHECK array State or State of FLOOR Call IF CALL FLOOR 3
    //WHEN switch toggle low, execute elevator move
    if (digitalRead(Switch_2) == LOW) {
      digitalWrite(FLOOR_2_LED, LOW);
      digitalWrite(FLOOR_1_LED, LOW);
      //UNTIL Elevator gets to Floor 2 LOOP
      while (digitalRead(Switch_3) != HIGH) {
        delay(1500);
      }
      //When Loop Breaks (AT FLOOR) EXECUTE FLOOR STATE
      if (digitalRead(Switch_3) == HIGH) {
        digitalWrite(FLOOR_3_LED, HIGH);
        Serial.println(F("FLOOR 3 REACHED"));
        //LOG FLOOR CALL DATA
        Data[index_state].DateTime = UpTimeTracker();
        Data[index_state].floorVisit = FloorCallTracker[0];
        index_state = (index_state + 1) % 150;
        //SHIFT FLOOR CALL REGISTERES LEFT FOR NEXT FLOOR CALL STATE
        if (index_floor_state != 0) {
          FloorCallTracker[0] = FloorCallTracker[1];
          FloorCallTracker[1] = FloorCallTracker[2];
        }
        //TRACKING DATA FOR CONFIRMED FLOOR CALL SHIFT REGITER
        Serial.println(F("NEXT FLOOR SHIFT REGISTER"));
        Serial.println(FloorCallTracker[0]);
      }

      //delay timer for passengers to board elevator before next floor call
      delay(1500);
    }
  }
} //end function 2

/*****Floor3 Function*****************************************
  Floor3 Function
  Accounts for all conditions while on the third floor
*************************************************************/
void Floor3Function(int FloorCallTracker[]) {
  //STATE tracking data
  Serial.println(F("index_floor_state"));
  Serial.println(index_floor_state);
  Serial.println(FloorCallTracker[0]);
  Serial.println(FloorCallTracker[1]);
  Serial.println(FloorCallTracker[2]);
  //FLOOR Set Status confirmation
  digitalWrite(FLOOR_1_LED, LOW);
  digitalWrite(FLOOR_2_LED, LOW);
  digitalWrite(FLOOR_3_LED, HIGH);

  //CHECK array State or State of FLOOR Call IF CALL FLOOR 2 or 1
  if (FloorCallTracker[0] == 2 || FloorCallTracker[0] == 1 ) {
    //WHEN switch toggle low, execute elevator move
    if (digitalRead(Switch_3) == LOW) {
      digitalWrite(FLOOR_3_LED, LOW);
      digitalWrite(FLOOR_1_LED, LOW);
      //UNTIL Elevator gets to Floor LOOP
      while (digitalRead(Switch_2) != HIGH) {
        delay(1500);
      }
      //When Loop Breaks (AT FLOOR) EXECUTE FLOOR STATE
      if (digitalRead(Switch_2) == HIGH) {
        digitalWrite(FLOOR_2_LED, HIGH);
        Serial.println(F("FLOOR 2 REACHED"));
        //BUGFIX CANT be FLOOR 2 AGAIN
        Data[index_state].DateTime = UpTimeTracker();
        Data[index_state].floorVisit = FloorCallTracker[0];
        index_state = (index_state + 1) % 150;
        //SHIFT FLOOR CALL REGISTERES LEFT FOR NEXT FLOOR CALL STATE
        if (FloorCallTracker[0] != 0) {
          FloorCallTracker[0] = FloorCallTracker[1];
          FloorCallTracker[1] = FloorCallTracker[2];
        }
        //TRACKING DATA FOR CONFIRMED FLOOR CALL SHIFT REGITER
        Serial.println(F("NEXT FLOOR SHIFT REGISTER"));
        Serial.println(FloorCallTracker[0]);
      }
      //delay timer for passengers to board elevator before next floor call
      delay(1500);
    }
  }
  //CHECK array State or State of FLOOR Call IF CALL FLOOR 3
  if (FloorCallTracker[0] == 3) {
    //WHEN switch toggle low, execute elevator move
    if (digitalRead(Switch_3) == LOW) {
      digitalWrite(FLOOR_2_LED, LOW);
      digitalWrite(FLOOR_1_LED, LOW);
      do {
        //When Loop Breaks (AT FLOOR) EXECUTE FLOOR STATE
        if (digitalRead(Switch_3) == HIGH) {
          digitalWrite(FLOOR_3_LED, HIGH);
          Serial.println(F("FLOOR 3 REACHED"));
          //BUGFIX CANT be FLOOR 2 AGAIN
          Data[index_state].DateTime = UpTimeTracker();
          Data[index_state].floorVisit = FloorCallTracker[0];
          index_state = (index_state + 1) % 150;
          //SHIFT FLOOR CALL REGISTERES LEFT FOR NEXT FLOOR CALL STATE
          if (index_floor_state != 0) {
            FloorCallTracker[0] = FloorCallTracker[1];
            FloorCallTracker[1] = FloorCallTracker[2];
          }
          //TRACKING DATA FOR CONFIRMED FLOOR CALL SHIFT REGITER
          Serial.println(F("NEXT FLOOR SHIFT REGISTER"));
          Serial.println(FloorCallTracker[0]);
        }
      } while (digitalRead(Switch_3) != HIGH);//{ //UNTIL Elevator gets to Floor LOOP

      //delay timer for passengers to board elevator before next floor call
      delay(1500);
    }
  }
} //end function 3

/*****Master Stop Function*****************************************
  Master Stop Function
  Interupts the elevator and send it to floor 1
******************************************************************/
void masterStop() {
  //MSTOP STATE TRACKER
  Serial.println(F("MSTOP pressed"));
  digitalWrite(MSTOP_LED, HIGH);
  //CLEAR STATE
  FloorCallTracker[0] = 0;
  FloorCallTracker[1] = 0;
  FloorCallTracker[2] = 0;
  index_floor_state = 0;
  //CHECK LATCH STATE OF FLOOR 1
  while (digitalRead(Switch_1) != HIGH) {
    //CHECK LATCH STATE OF ALL FLOOR
    if (digitalRead(Switch_3) == HIGH && digitalRead(Switch_2) == LOW && digitalRead(Switch_1) == LOW) {
      //SET FLOOR STATE
      index_floor_state = 2;
      FloorCallTracker[0] = 2;
      FloorCallTracker[1] = 1;
      FloorCallTracker[2] = 0;
      Floor3Function(FloorCallTracker);
    }  if (digitalRead(Switch_2) == HIGH && digitalRead(Switch_1) == LOW && digitalRead(Switch_3) == LOW) { //CHECK LATCH STATE OF ALL FLOOR
      //SET FLOOR STATE
      index_floor_state = 1;
      Floor2Function(FloorCallTracker);
    }  if (digitalRead(Switch_1) == HIGH && digitalRead(Switch_2) == LOW && digitalRead(Switch_3) == LOW) { //CHECK LATCH STATE OF ALL FLOOR
      //SET FLOOR STATE
      Floor1Function(FloorCallTracker);
    }
  }
  delay(1500);
  digitalWrite(MSTOP_LED, LOW);
  //detachInterrupt (masterStop);
} //end function

/*****Emergency Stop Function**************************************
  Emergency Stop Function
  Interupts the elevator and keeps it at the current floor
******************************************************************/
void emergencyStop() {
  //ESTOP STATE TRACKER
  Serial.println(F("ESTOP pressed"));
  digitalWrite(ESTOP_LED, HIGH);
  //CLEAR STATE of CALLS
  FloorCallTracker[0] = 0;
  FloorCallTracker[1] = 0;
  FloorCallTracker[2] = 0;
  index_floor_state = 0;
  //loop until emergency fault cleared
  while (digitalRead(Floor_1_ELEVATOR_Button) != HIGH) {
    digitalWrite(ESTOP_LED, HIGH);
  }
  delay(1500);
  digitalWrite(ESTOP_LED, LOW);
  //detachInterrupt (emergencyStop);
} //end function

/*****Serial Monitor Function*************************************
  Serial Monitor STATUS for Up-Time tracker
******************************************************************/
void SerialMonitor () {
  if (Serial.available() > 0) {
    incoming = Serial.read();
    {
      if (incoming == 63) // if ? received then answer with data
      {
        delay(1500);
        currentmillis = millis(); // get the  current milliseconds
        // print milliseconds
        Serial.print("Total milliseconds running: ");
        Serial.println(currentmillis);
        UpTimeTracker(); //call conversion function
      }
    }
  }
} //end function

/*****UpTimeTracker Function**************************************
  Tracks up time with serial monitor
  TIME DATE CONVERSIONS IN THIS FUNCTION
******************************************************************/
String UpTimeTracker() {
  delay (1500);
  //TIME DATE CONVERSIONS
  Seconds = currentmillis / 1500; //convect milliseconds to seconds
  Minutes = Seconds / 60; //convert seconds to minutes
  Hours = Minutes / 60; //convert minutes to Hours
  Days = Hours / 24; //convert Hours to Days
  Seconds = Seconds - (Minutes * 60); //subtract the coverted seconds to minutes
  Minutes = Minutes - (Hours * 60); //subtract the coverted minutes to Hour
  Hours = Hours - (Days * 24); //subtract the coverted Hours to Days

  //Display results
  String stringThree = "";
  String stringFour = "";
  String stringOne = String("Running Time ");

  // Days will displayed only if value is greater than zero
  if (Days > 0)
  {
    stringThree = String(Days);
    stringFour = String(" Days and :");
  }

  //PRINT LOGS AT THE END OF EACH DAY SERIALLY
  if (Hours == 23 && Minutes < 50) {
    PrintLogs();
  }

  //RESET LOGS AFTER 7 DAYS
  if (Days == 7) {
    //CLEARS THE DATA TRACKER ARRAY FROM MEMORY
    memset(Data, 0, sizeof(Data));
    //RESETS the UpTime Tracker
    Days = 0;
    Hours = 0;
    Minutes = 0;
    Seconds = 0;
  }
  //SET CONVERSIONS UP IN STRING OF VALUES
  String stringFive = String(Hours);
  String stringSix = String(":");
  String stringSeven = String(Minutes);
  String stringEight = String(":");
  String stringNine = String(Seconds);

  //STRING OF DATE HOURS/MINS/SEC for EASY STORAGE AND PRINTING
  String DateTime = String(stringOne + stringThree + stringFour + stringFive + stringSix + stringSeven + stringEight + stringNine);
  Serial.println(DateTime);
  return DateTime;
} //end function
/*****Print Log Function**************************************
  When Elevator Button 1 and 3 are pressed
  This will dump maintenance logs
******************************************************************/
void PrintLogs() {
  //LOOP through VALUES of array printing data
  for (int i = 0; i < 150; i++) {
    Serial.println(F("Retrieving sensor data..."));
    Serial.print (F("for time: "));
    Serial.print(Data[i].DateTime);
    Serial.println(F(" the floorVisit at the time was "));
    Serial.println(Data[i].floorVisit);
    Serial.println(F("Print Logs"));
    delay(1500);
    //IF CONDITION TO BREAK LOOK BEFORE 150 VALUES
    if (i == index_state) {     // bail out on sensor detect
      index_floor_state = 0;
      FloorCallTracker[0] = 0;
      FloorCallTracker[1] = 0;
      FloorCallTracker[2] = 0;
      break;
    }
  }
} //end function

/*****WelcomeFunction*****************************************
  Welcome Screen
  Boot Message
************************************************************/
void Welcome() {
  Serial.println(F("####### #       ####### #     #    #    ####### ####### ######                  "));
  Serial.println(F("#       #       #       #     #   # #      #    #     # #     #                 "));
  Serial.println(F("#       #       #       #     #  #   #     #    #     # #     #                 "));
  Serial.println(F("#####   #       #####   #     # #     #    #    #     # ######                  "));
  Serial.println(F("#       #       #        #   #  #######    #    #     # #   #                   "));
  Serial.println(F("#       #       #         # #   #     #    #    #     # #    #                  "));
  Serial.println(F("####### ####### #######    #    #     #    #    ####### #     #                 "));
  Serial.println(F("                                                                                "));
  Serial.println(F(" #####  ####### #     # ####### ######  ####### #       #       ####### ######  "));
  Serial.println(F("#     # #     # ##    #    #    #     # #     # #       #       #       #     # "));
  Serial.println(F("#       #     # # #   #    #    #     # #     # #       #       #       #     # "));
  Serial.println(F("#       #     # #  #  #    #    ######  #     # #       #       #####   ######  "));
  Serial.println(F("#       #     # #   # #    #    #   #   #     # #       #       #       #   #   "));
  Serial.println(F("#     # #     # #    ##    #    #    #  #     # #       #       #       #    #  "));
  Serial.println(F(" #####  ####### #     #    #    #     # ####### ####### ####### ####### #     # "));
} //end function
