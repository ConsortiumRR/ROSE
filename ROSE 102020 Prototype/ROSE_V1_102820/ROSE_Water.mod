MODULE ROSE_Water
    !***********************************************************
    !
    ! Module:  Rose 102820 Watering Proto Test

    ! Description:
    !   This program queries a "Hub" microcontroller via IO signal to send an ID number for a plant that needs to be watered. Hub gathers this info from IOT network of sensors installed at each plant.
    ! Robot receives the plant id number, picks up a watering end effector, and moves to the defined location for that plant - triggering a water pump. Robot then returns to start position to wait for additional instruction. 
    !
    ! Author: Phoebe
    ! Version: 1.0 - Initial Offline Code 
    !>>> OFFLINE CODE - tool, location and watering target needs to be defined before run!
    !>>ToDo: Review RAPID syntax for PulseDO and TOol Load methods. Add in check/load tool procedure, init program setup, debug and tune
    !
    !>Future: split up into multiple modules eg generic functions vs variables? use interrupt receiver method?
    !***********************************************************

    !Tool Data >> to be defined!
    PERS tooldata GRIPPER:=[TRUE,[[-11.7954,-1.80928,241.823],[1,0,0,0]],[4,[0,0,10],[1,0,0,0],0,0,0]];
    PERS tooldata WATERING:=[TRUE,[[-11.7954,-1.80928,241.823],[1,0,0,0]],[4,[0,0,10],[1,0,0,0],0,0,0]];
    PERS tooldata PLANTER:=[TRUE,[[-11.7954,-1.80928,241.823],[1,0,0,0]],[4,[0,0,10],[1,0,0,0],0,0,0]];
    !Mutable tool for potential tool agnostic functions 
    PERS tooldata ANY_TOOL:=[TRUE,[[-11.7954,-1.80928,241.823],[1,0,0,0]],[4,[0,0,10],[1,0,0,0],0,0,0]];
    
    !Safe position between routines
    PERS jointtarget StartPos:=[[135.173,-20.0224,26.0093,1.74995,33.5196,-13.9474],[20.1663,9E+09,9E+09,9E+09,9E+09,9E+09]];
    
    !rough variables for tool loading and checking >> Need to review if there are pre-existing methods for tool loading, populate library array from program data directly?
    PERS num CurrentTool;
    PERS string ToolLibrary{3}:=["GRIPPER", "WATERING","PLANTER"];
    VAR signaldo Grip_DO;
    !location for watering tool pickup > could instead define as wobj? something else?
    PERS robtarget PickTool_Water:=[[-19.27,29.6,16.83],[0.00233746,0.744192,-0.667961,-0.000735195],[1,-1,0,0],[3299.84,9E+09,9E+09,9E+09,9E+09,9E+09]];

    ! some generic variables for position offsets and speeds
    CONST num safe:=100;
    CONST num pre:=50;
    PERS speeddata vfast:=[1000,500,1000,15];
    PERS speeddata vmed:=[250,100,200,15];
    PERS speeddata vslow:=[100,30,200,15];

    !switch locations as workobjects- a string array of the location names which allows for different naming conventions or additional data inclusion
    PERS string locations{8}:=["unit1","unit2","unit3","unit4","unit5","unit6","unit7","unit8"];
    !!!-->These need to be defined before the program is run - origin at the off position of the switch, +Y axis is the direction of throw
    PERS wobjdata unit1:=[FALSE,TRUE,"",[[-1776.04,1361.21,11.6311],[0.904913,0.000696189,-0.00226035,0.42559]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit2:=[FALSE,TRUE,"",[[2359.13,2559.65,943.899],[0.693407,0.720438,0.012368,0.00179935]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit3:=[FALSE,TRUE,"",[[420.344,-1733.52,303.799],[0.000861603,0.0116399,0.00291137,-0.999928]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit4:=[FALSE,TRUE,"",[[-80.0228,1668.95,-315.727],[0.918385,-0.0017154,-0.0118785,0.395506]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit5:=[FALSE,TRUE,"",[[3299.91,1808.63,590.686],[0.919693,0.00732166,-0.0011623,-0.392568]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit6:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit7:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata unit8:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];

    !Watering data - target relative to wobj def, DO for pump, time in seconds pump will be powered
    PERS robtarget watertarg:=[[-19.27,29.6,16.83],[0.00233746,0.744192,-0.667961,-0.000735195],[1,-1,0,0],[3299.84,9E+09,9E+09,9E+09,9E+09,9E+09]];
    VAR signaldo Pump_DO; 
    VAR num Pump_Time:=5; 
    
    !This is a temporary wobj variable used for storing the next wobj the robot will use - ? could be swapped as a variable instead? 
    PERS wobjdata nextunit;
    !a counter for how many switches were flipped
    VAR num counter:=0;

    !Variables for receiving data from Hub- signal name and signal read variable,  array of pulse lengths for sending 4 different "inquiries"
    VAR num Inputnum;
    VAR signalgi HubByteIn;
    VAR signaldo HubPulseOut;
    PERS num pulselengths{4}:=[0.005,0.010,0.015,0.020];

    !named interrupt for using interupt receive method
    VAR intnum HubByteReceived;
    !a value we can use as a max time to wait before throwing an error in seconds
    PERS num maxsignalwait:=360;


    !***********************************************************
    PROC main()
        !Turn configuration control off and clear Teach Pendant
        ConfL\Off;
        ConfJ\Off;
        TPErase;
        !Set the addresses for various signals >> could be done in a diff initialization module or something
        AliasIO GI_0108,HubByteIn;
        AliasIO DO_02,HubPulseOut;
        AliasIO DO_03, Pump_DO;
        AliasIO DO_04, Grip_DO; 
        !set the signal power do to high
        SetDO DO_01,1;

        !*Could add a TP query to trigger optional routine to setup connection and/or teach targets
        !*Option to add an interupt that reads the signal at any time it is received
        !/CONNECT HubByteReceived WITH ReadSignal; 
        !/ISignalGI HubByteIn, HubByteReceived;

        !Retract from current position at a medium speed and move to start position
        SafeRetract vmed;
        MoveAbsJ StartPos,vfast,z30,tool0;

        !Run Interaction Procedure
        TPWrite "Hello! Welcome to R/OSE";
        reg1:=1;
        Interact;
        !End program
        TPWrite ""+NumToStr(counter,1)+" plants have been watered";
        TPWrite "Exiting R/OSE program. Goodbye!";
    ENDPROC

    PROC Interact()
        !bookmarks for goto commmands >need to review rapid manual to see limitations
        startinteract:

        !reg1 = 1 as long as it hasn't been changed due to an error or time out- so the loop should continue indefinitely 
        !>this could be replaced with a program end command in the error handler
        WHILE reg1=1 DO
            !Wait until the robot is in position after retract
            WaitRob\Inpos;
            !Get the signal from the hub for the switch location
            MessageHandshake;
            !Turn off the desired switch
            IF Inputnum<>0 AND Inputnum<=Dim(locations,1) THEN
                WATERPLANT;
            ENDIF
            !Safe Retract and set inputnum back to 0
            SafeRetract vfast;
            Inputnum:=0;
        ENDWHILE
        endinteract:

    ENDPROC

    PROC WATERPLANT()
        !ADD>> Check and Load Watering Tool 
        
        !Option to safely move the robot to the closest track position first
        SafeTrack nextunit,watertarg;

        !Approach the plant > might be able to do this in 1 step
        MoveJ Offs(watertarg,0,0,safe),vfast,z40,WATERING\WObj:=nextloc;
        MoveL Offs(watertarg,0,0,pre),vfast,z30,WATERING\WObj:=nextloc;
        !Flips the switch
        MoveL watertarg,vslow,z5,WATERING\WObj:=nextloc;
        RUNPUMP; 
        !Retracts from switch
        MoveL Offs(watertarg,0,0,safe),vfast,z30,WATERING\WObj:=nextloc;
        !increments the counter and writes data to TP
        INCR counter;
        TPwrite "Plant Unit "+locations{inputnum}+"was watered.";
    ENDPROC
    
    PROC RUNPUMP()
        !waits until robot in position, turns on pump power for set time >>tune pump on time
        WaitRob \InPos; 
        PulseDO \Plength:=Pump_Time, Pump_DO;
        !waits 5 seconds after pump is off to ensure water is done flowing 
        !>>>Need to review RAPID manual regarding how pulse works with waits
        WaitDO Pump_DO, 0; 
        WaitTime 5;
    ENDPROC
    
    PROC LOADTOOL(INOUT num toolnum)
        !Receives desired tool number checks for that tool and loads it if not loaded
        !> could split to a check tool and a load tool procedure?
        
        IF CurrentTool=toolnum THEN
            !check if desired tool is loaded , if yes return
            RETURN;
        ELSEIF CurrentTool<>1 THEN
            !check if any tool (other than gripper) is loaded, if yes unload the tool 
            UNLOADTOOL CurrentTool;
        ENDIF
        
        !if no tool is loaded go to the tool pick/drop location 
        SetDO Grip_DO,0; 
        MoveJ Offs(pickTool_Water,0,0,safe),vmed,z40,GRIPPER;
        MoveL PickTool_Water,vslow,z10,GRIPPER;
        !wait until rob in position and release grip
        WaitRob \InPos; 
        GRIPCLOSE;
        !retract and set CurrentTool to the called toolnum
        MoveL Offs(pickTool_Water,0,-safe,0),vmed,z40,GRIPPER;
        MoveAbsJ StartPos, vfast, z40, GRIPPER;
        CurrentTool:= toolnum;
    ENDPROC
    
    PROC UNLOADTOOL(inout num toolnum)
        !Unloads current tool
        !>>Need to change the target to mutable pick/drop location and tune approach direction
        
        !Approach pickdrop location for given tool 
        MoveJ Offs(pickTool_Water,0,0,safe),vmed,z40,GRIPPER;
        MoveL PickTool_Water,vslow,z10,GRIPPER;
        !open gripper
        WaitRob \InPos;
        GRIPOPEN;
        !retract and set toolnum to 1 (gripper)
        MoveL Offs(pickTool_Water,0,-safe,0),vmed,z40,GRIPPER;
        MoveAbsJ StartPos, vfast, z40, GRIPPER;
        CurrentTool:=1;
    ENDPROC
    
    PROC GRIPOPEN()
        SetDO Grip_DO, 0;
        WaitTime 1;
    ENDPROC
    
    PROC GRIPCLOSE()
        SetDO Grip_DO, 1;
        WaitTime 1;
    ENDPROC

    PROC MessageHandshake()
        !a temporary label to allow us to pull stuff from the array of location names
        VAR wobjdata locationlabel;

        starthandshake:

        !Pulse output to send a "ready" message to the Hub
        PulseDO\PLength:=pulselengths{1},HubPulseOut;

        ! Waits for hub signal to not equal 0- if the wait time exceeds max wait time it will throw an error
        WaitGI HubByteIn,\NOTEQ,0,\MaxTime:=maxsignalwait;

        !Wait for 5ms to ensure a stable signal then set the input num to value of the signal
        WaitTime .005;
        Inputnum:=HubByteIn;
        !TPReadNum Inputnum, "write TEST inputnum";

        IF Inputnum=0 THEN
            !If the input is somehow 0 then start the message handshake again
            GOTO retryshake;
            TPWrite "no input received";
        ELSEIF Inputnum>=DIM(locations,1) THEN
            !If the input is higher than the number of stored positions than write to tp and start handshake again
            TPWrite "Invalid location from hub:"\Num:=Inputnum;
            Inputnum:=0;
            GOTO retryshake;
        ELSE
            !If the inputnum is valid then get the location name stored at that position in the location array
            GetDataVal locations{Inputnum},locationlabel;
            !now set the next switch target location to the one called by the location label
            !this is a bit of a workaround due to wobjs not being able to be stored in arrays with labels
            nextunit:=locationlabel;
            TPWrite "Going to water unit "+locations{Inputnum};
        ENDIF
        endhandshake:
        RETURN ;
        retryshake:

        !Error handling for exceeding the max wait time- this will end the program if the user does not click continue
    ERROR
        IF ERRNO=ERR_WAIT_MAXTIME THEN
            !If the max wait time error is thrown the TP shows the option to continue
            TPWrite "No signal received from Hub for "+NumToStr(maxsignalwait,1)+" seconds";
            reg1:=0;
            !reg1 stores the response of the function key the user pressed
            TPReadFK reg1,"Would you like to continue?","Yes","No",stEmpty,stEmpty,stEmpty;
            IF reg1=1 THEN
                ! If the user selects yes (reg1=1) it begins the handshake sequence again
                TRYNEXT;
            ELSE
                !If the user selects no it sends the cursor to the end of the current procedure and should cause the program to end
                TPWrite "Ending program";
                RETURN ;
            ENDIF
        ELSE
            !not sure if anything needs to go here
        ENDIF

        ! this returns to the interact having set a new nextloc workobject
    ENDPROC

    PROC SafeRetract(INOUT speeddata speed)
        !This procedure retracts the robot from its current position to the middle of the track to prevent collisions
        !*could add way to retract in a specific direction or turn on joint supervision? 
        !MotionSup \On \TuneValue:=50; !Motion supervision tune set to twice as sensitive as normal
        VAR jointtarget Cjoints;
        Cjoints:=CJointT();
        MoveAbsJ [[Cjoints.robax.rax_1,-30,30,0,30,0],[Cjoints.extax.eax_a,9E+09,9E+09,9E+09,9E+09,9E+09]]\NoEOFFs,speed,z50,tool0;
        !MotionSup \On \TuneValue:=100; !set tune back to normal
    ENDPROC

    PROC SafeTrack(INOUT wobjdata nextwobj,INOUT robtarget nexttarg)
        !* I realize now there is a RAPID method  for independent axis movement - IndAMove which may work
        !This takes the next target and moves the robot safely to the closes track position   

        !create a joint target for the desired next track position with retract
        VAR jointtarget trackpos:=[[0,-30,30,0,30,0],[0,9E+09,9E+09,9E+09,9E+09,9E+09]];
        !temporary variables for the current joint position and joint position of the 
        VAR jointtarget Cjoints;
        VAR jointtarget Nextjoints;
        VAR robtarget Ctarg;
        VAR num xcord;
        VAR num theta;
        !Calculate the current joint position and the joint equivalent of the next target
        Cjoints:=CJointT();
        Ctarg:=CRobT(\WObj:=wobj0);


        !get the x offset (along the track) of the next switch location
        xcord:=nextwobj.uframe.trans.x;

        IF Abs(Cjoints.extax.eax_a-xcord)>1000 THEN
            IF xcord<=0 THEN
                !if the x coord is less than or equal to 0 set the track to go to 0
                trackpos.extax.eax_a:=0;
            ELSEIF xcord>=9700 THEN
                !if the x coord is greater or equal to 9700 (track length) the track should go to 9700
                trackpos.extax.eax_a:=9700;
            ELSE
                !if the xcoord is somewhere in between the track should go to the x coordinate of the target
                trackpos.extax.eax_a:=xcord;
            ENDIF
        ELSE
            !if xcord is within 1 meter of the current location leave track position as is
            trackpos.extax.eax_a:=Cjoints.extax.eax_a;
        ENDIF
        !trig 
        theta:=Atan2(nextwobj.uframe.trans.y,(xcord-trackpos.extax.eax_a));
        TPWrite "" \Num:=theta;
        trackpos.robax.rax_1:=theta;
        !Set new axis1 pos to the next targ
        !Now move to the new track position
        nexttarg.extax.eax_a:=trackpos.extax.eax_a;
        !Nextjoints := CalcJointT(nexttarg, WATERING \Wobj:=nextwobj);
        MoveAbsJ trackpos,vfast,z30,tool0;
        !make the track position of the next targ robtarget the same as the safe trackposition
    ENDPROC

    !This trap can be used for reading signals as they are sent instead of during signal handshake needs to be enabled in main
    TRAP ReadSignal
        Inputnum:=HubByteIn;
        RETURN ;
    ENDTRAP
ENDMODULE