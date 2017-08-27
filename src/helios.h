#ifndef HELIOS_H
#define HELIOS_H

//*****************************************************************************
//$Workfile: helios.h
//  Authors: Peter Nagy <peter.nagy@perit.hu>
// Reviewer: -
//  Project: Bypass control correction of a Helios ventilation system
//     Date: 24.08.2017
//*****************************************************************************
// Description:
//   ...
//*****************************************************************************
// COPYRIGHT (C) 2017, Peter Nagy
//
// This file is part of a home automation project "Helios bypass control".
//
// Helios bypass control is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Helios bypass control is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with the project.  If not, see <http://www.gnu.org/licenses/>.
//*****************************************************************************


//*** INCLUDES ****************************************************************

#include <modbus.h>
#include <string>
#include <exception>

//*** DEFINES AND CONSTANTS ***************************************************
#define LOGERROR(fmt,...) fprintf (stderr, "%s error: " fmt "\n", _getTimestamp().c_str(), ##__VA_ARGS__)

//*** TYPES AND CLASSES *******************************************************
std::string _getTimestamp();

class HeliosException : public std::exception
{
    //------- Enums, Typedefs -------

    //------- Attribute -------------
    protected:
        std::string     myErrorMsg;

    //------- Services --------------
    public:
        // Constructor
                        HeliosException( const std::string errorMsg )
                        {
                            myErrorMsg = errorMsg;
                        };


        // Destructor
        virtual         ~HeliosException()
                        {

                        };

        virtual const char* what()
                        {
                            return myErrorMsg.c_str();
                        }
};


//---------------------------------------------------------------------
//  Definition of the class HeliosKWLEC370WR
//---------------------------------------------------------------------

// Description ......
//
//
//
//

class HeliosKWLEC370WR
{
    //------- Enums, Typedefs -------
    protected:
        static const char*  IPADDRESS;
        static const char*  PORT;
        static const int    SLAVEID = 180; // This is the set slave id of the Helios on the modbus
        static const int    TIMEOUT = 2; // 2 sec


    //------- Attribute -------------
    protected:
        modbus_t*       myModbusPtr = NULL;
        bool            myBypassState = false; // false: closed; true: open

    //------- Services --------------
    public:
        // Constructor
                        HeliosKWLEC370WR();

        // Destructor
        virtual         ~HeliosKWLEC370WR();

        virtual std::string GetVariable( std::string varName );
        virtual void    SetVariable( std::string varName, double value );
        virtual void    ModulTest();
        virtual void    ControlBypassValve();

    protected:
        virtual void    _openModbus();
        virtual void    _closeModbus();
        virtual double  _getAirTemperature( int ventilNr ); // 1: outdoor; 2: supply; 3: extract; 4: exhaust
        virtual double  _getBypassMinOutdoorTemp();
        virtual void    _setBypassRoomTemperature( double roomTemp );
        virtual void    _openBypass();
        virtual void    _closeBypass();
        virtual void    _swapBytes( char* data, int len );
};

#endif // HELIOS_H
