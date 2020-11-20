MODULE CPXColorTest
    !***********************************************************
    !
    ! Module:  CPXColorTest
    ! Description:
    !   This program sends a binary output from 0 to 16 to the ESP32 hub which passes it via UART to a circuit playground which displays the corresponding color output
    !
    ! Author: Phoebe
    ! Version: 1.3 setup for semi-permanent install on the 1600
    !
    !***********************************************************


    !Variables for receiving data from Hub- signal name and signal read variable
    VAR dnum Inputnum;
    VAR signalgi HubByteIn;
    VAR signaldo InputPower;
    VAR signalgo HubByteOut; 

    !Variables for sending pulse inquiry to Hub- array of pulse lengths for sending 4 different "messages"
    VAR signaldo HubPulseOut;
    PERS num pulselengths{4}:=[0.005,0.010,0.015,0.020];

    !named interrupt if we plan on using that method
    VAR intnum HubByteReceived;
    !a value we can use as a max time to wait before throwing an error in seconds
    PERS num maxsignalwait:=360;
    VAR num check;
    PERS string colorlib{15}:=["OFF", "WHITE", "RED","PINK", "MAGENTA", "VIOLET", "BLUE", "AZURE", "CYAN", "AQUA", "GREEN", "LIME", "YELLOW", "ORANGE", "SIENNA"];


    !***********************************************************
    PROC main()
        !Turn configuration control off and clear Teach Pendant
        ConfL\Off;
        ConfJ\Off;
        TPErase;
        AliasIO GI_0108,HubByteIn;
        AliasIO DSQ651_10_DO_01,InputPower;
        AliasIO GO_0310, HubByteOut; 
        !set the signal power do to high
        SetDO InputPower,1;

        !Run Interaction Procedure
        TPWrite "Hello!";
        check:=1;
        Interact;
        !End program

    ENDPROC

    PROC Interact()
        !these commands with colons are labels that function as "bookmarks" for the GOTO command
        startinteract:

        !check = 1 as long as it hasn't been changed due to an error or time out- so the loop should continue indefinitely 
        !this could be replaced with a program end command in the error handler
        WHILE check=1 DO
            !Ask user to give a color
            TPWrite "Color Library: OFF:1, WHITE:2, RED:3, ROSE:4, MAGENTA:5, VIOLET:6, BLUE:7,";
            TPWrite "AZURE:8, CYAN:9, AQUA:10, GREEN:11, LIME:12, YELLOW:13, ORANGE:14, SIENNA:15";
            TPReadDnum Inputnum, "Select color command:";
            !Turn off the desired switch
            IF Inputnum>0 AND Inputnum<=15 THEN
                SetGO HubByteOut, (Inputnum-1);
                TPWrite "You selected" + colorlib{DnumToNum(inputnum)}; 
            ELSE
                TPWrite "Input out of bounds - select again";
            ENDIF
        ENDWHILE
        endinteract:

    ENDPROC


ENDMODULE