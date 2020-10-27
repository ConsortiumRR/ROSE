MODULE UselessMachine
    !***********************************************************
    !
    ! Module:  UselessMachine 

    ! Description:
    !   This program queries a Hub microcontroller via IO to tell it when a toggle switch (connected to a separate microcontroller) is flipped on 
    !   It then moves to turn off the switch at that location and repeats the program
    !
    ! Author: Phoebe
    ! Version: 1.2 - setup 09/21/20
    !
    !***********************************************************

    !-->Setup Variables for Gripper and Safe Area
    PERS tooldata TOOL1:=[TRUE,[[-11.7954,-1.80928,241.823],[1,0,0,0]],[4,[0,0,10],[1,0,0,0],0,0,0]];
    PERS jointtarget StartPos:=[[135.173,-20.0224,26.0093,1.74995,33.5196,-13.9474],[20.1663,9E+09,9E+09,9E+09,9E+09,9E+09]];

    ! some generic variables for speed and offsets
    CONST num safe:=100;
    CONST num pre:=50;
    PERS speeddata vfast:=[1000,500,1000,15];
    PERS speeddata vmed:=[250,100,200,15];
    PERS speeddata vslow:=[100,30,200,15];

    !switch locations as workobjects- a string array of the location names which allows for different naming conventions or additional data inclusion
    PERS string locations{8}:=["loc1","loc2","loc3","loc4","loc5","loc6","loc7","loc8"];
    !!!-->These need to be defined before the program is run - origin at the off position of the switch, +Y axis is the direction of throw
    PERS wobjdata loc1:=[FALSE,TRUE,"",[[-1776.04,1361.21,11.6311],[0.904913,0.000696189,-0.00226035,0.42559]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc2:=[FALSE,TRUE,"",[[2359.13,2559.65,943.899],[0.693407,0.720438,0.012368,0.00179935]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc3:=[FALSE,TRUE,"",[[420.344,-1733.52,303.799],[0.000861603,0.0116399,0.00291137,-0.999928]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc4:=[FALSE,TRUE,"",[[-80.0228,1668.95,-315.727],[0.918385,-0.0017154,-0.0118785,0.395506]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc5:=[FALSE,TRUE,"",[[3299.91,1808.63,590.686],[0.919693,0.00732166,-0.0011623,-0.392568]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc6:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc7:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];
    PERS wobjdata loc8:=[FALSE,TRUE,"",[[11983.2,-783.529,365.386],[0.309256,-0.0023921,0.00743437,-0.950947]],[[0,0,0],[1,0,0,0]]];

    !!!-->not sure on this target - it should probably be roughly at 0,0,0 facing down - track may need to be defined?
    PERS robtarget switchoff:=[[-19.27,29.6,16.83],[0.00233746,0.744192,-0.667961,-0.000735195],[1,-1,0,0],[3299.84,9E+09,9E+09,9E+09,9E+09,9E+09]];

    PERS wobjdata nextloc;
    !This is a temporary wobj variable used for storing the next wobj the robot will use
    VAR num counter:=0;
    !a counter for how many switches were flipped

    !Variables for receiving data from Hub- signal name and signal read variable
    VAR num Inputnum;
    VAR signalgi HubByteIn;

    !Variables for sending pulse inquiry to Hub- array of pulse lengths for sending 4 different "messages"
    VAR signaldo HubPulseOut;
    PERS num pulselengths{4}:=[0.005,0.010,0.015,0.020];

    !named interrupt if we plan on using that method
    VAR intnum HubByteReceived;
    !a value we can use as a max time to wait before throwing an error in seconds
    PERS num maxsignalwait:=360;


    !***********************************************************
    PROC main()
        !Turn configuration control off and clear Teach Pendant
        ConfL\Off;
        ConfJ\Off;
        TPErase;
        AliasIO GI_0108,HubByteIn;
        AliasIO DO_02,HubPulseOut;
        !set the signal power do to high
        SetDO DO_01,1;

        !*Could add a TP query to trigger optional routine to setup connection and/or teach targets
        !*Option to add an interupt that reads the signal at any time it is received
        !/CONNECT HubByteReceived WITH ReadSignal; 
        !/ISignalGI HubByteIn, HubByteReceived;

        !Retract from current position at a medium speed and move to start position
        SafeRetract vmed;
        !MoveAbsJ StartPos,vfast,z30,tool0;

        !Run Interaction Procedure
        TPWrite "Hello!";
        reg1:=1;
        Interact;
        !End program
        TPWrite ""+NumToStr(counter,1)+" switches were flipped";
        TPWrite "Goodbye!";
    ENDPROC

    PROC Interact()
        !these commands with colons are labels that function as "bookmarks" for the GOTO command
        startinteract:

        !reg1 = 1 as long as it hasn't been changed due to an error or time out- so the loop should continue indefinitely 
        !this could be replaced with a program end command in the error handler
        WHILE reg1=1 DO
            !Wait until the robot is in position after retract
            WaitRob\Inpos;
            !Get the signal from the hub for the switch location
            MessageHandshake;
            !Turn off the desired switch
            IF Inputnum<>0 AND Inputnum<=Dim(locations,1) THEN
                TurnOff;
            ENDIF
            !Safe Retract and set inputnum back to 0
            SafeRetract vfast;
            Inputnum:=0;
        ENDWHILE
        endinteract:

    ENDPROC

    PROC TurnOff()
        !Option to safely move the robot to the closest track position first
        SafeTrack nextloc,switchoff;

        !Approach the switch from an offset above and away from the "on" direction
        MoveJ Offs(switchoff,0,-safe,safe),vfast,z40,TOOL1\WObj:=nextloc;
        MoveL Offs(switchoff,0,-pre,0),vfast,z30,TOOL1\WObj:=nextloc;
        !Flips the switch
        MoveL switchoff,vslow,z5,TOOL1\WObj:=nextloc;
        !Retracts from switch
        MoveL Offs(switchoff,0,-safe,safe),vfast,z30,TOOL1\WObj:=nextloc;
        !increments the counter and writes data to TP
        INCR counter;
        TPwrite "switched off "+locations{inputnum}+". "+NumToStr(counter,1)+" switches have been flipped";
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
            nextloc:=locationlabel;
            TPWrite "Going to switch off "+locations{Inputnum};
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
        !Nextjoints := CalcJointT(nexttarg, TOOL1 \Wobj:=nextwobj);
        MoveAbsJ trackpos,vfast,z30,tool0;
        !make the track position of the next targ robtarget the same as the safe trackposition
    ENDPROC

    !This trap can be used for reading signals as they are sent instead of during signal handshake needs to be enabled in main
    TRAP ReadSignal
        Inputnum:=HubByteIn;
        RETURN ;
    ENDTRAP
ENDMODULE