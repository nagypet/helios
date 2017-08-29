//*****************************************************************************
//$Workfile: helios.cpp
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

#include <string.h>
#include <errno.h>
#include <string>
#include <sysio/defs.h>
#include "helios.h"
#include "processdata.h"

const char* HeliosKWLEC370WR::IPADDRESS = "192.168.128.40";
const char* HeliosKWLEC370WR::PORT = "502";


HeliosKWLEC370WR::HeliosKWLEC370WR() noexcept
{
    myModbusPtr = NULL;
    myBypassState = false;
    myOutdoorTemp = 0;
    myRoomTemp = 0;
    myMinOutdoorTemp = 0;
    myOutdoorTemp.SetHysteresis( 0.5 );
    myRoomTemp.SetHysteresis( 0.5 );
    myMinOutdoorTemp.SetHysteresis( 0.5 );
}


HeliosKWLEC370WR::~HeliosKWLEC370WR()
{
    _closeModbus();
}


void HeliosKWLEC370WR::ModulTest()
{
    //_getAirTemperature( 1 );

    //std::string retVal = GetVariable( "v00000" );

    //ControlBypassValve();
    ProcessData data( 0 );
    data.SetHysteresis( 0.5 );
    data = 3.14;
    printf( "%.1f\n", data.Value());
}


// varName e.g. "v00104"

std::string HeliosKWLEC370WR::GetVariable( std::string varName )
{
    int iRet = 0;
    const int iReqLen = 4;
    const int iRespLen = 64;

    if( varName.length() != 6 || varName[0] != 'v' )
    {
        throw HeliosException( "Invalid parameter to HeliosKWLEC370WR::GetVariable()");
    }

    try
    {
        _openModbus();

        // writeBuffer shall contain the ASCII representation of the parameter number in big endian
        uint16_t writeBuffer[ iReqLen ];
        char* writeData = (char*)writeBuffer;
        memset( writeBuffer, 0, iReqLen * 2 );
        strcpy( writeData, varName.data() );
        _swapBytes( writeData, iReqLen * 2 );

        iRet = modbus_write_registers( myModbusPtr, 1, iReqLen, writeBuffer);

        if ( iRet == iReqLen )
        {
            PDEBUG("Written %d references.", iReqLen );
        }
        else
        {
            //fprintf (stderr, "Write %s failed: %s\n", (char*)writeData, modbus_strerror (errno));
            std::string errorMsg = "Write ";
            errorMsg += (char*)writeData;
            errorMsg += " failed: ";
            errorMsg += modbus_strerror (errno);
            throw HeliosException( errorMsg );
        }

        uint16_t readBuffer[ iRespLen ];
        char* readData = (char*)readBuffer;
        iRet = modbus_read_registers( myModbusPtr, 1, iRespLen, readBuffer );

        if ( iRet == iRespLen )
        {
            PDEBUG("Received %d references.", iRespLen );
        }
        else
        {
            //fprintf (stderr, "Read failed: %s\n", modbus_strerror (errno));
            std::string errorMsg = "Read failed: ";
            errorMsg += modbus_strerror (errno);
            throw HeliosException( errorMsg );
        }

        _swapBytes( readData, iRespLen * 2 );

        PDEBUG( "%s", readData );

        _closeModbus();

        return std::string( readData + 7 );
    }
    catch( HeliosException& e )
    {
        _closeModbus();
        throw e;
    }
}


void HeliosKWLEC370WR::SetVariable( std::string varName, double value )
{
    int iRet = 0;
    const int iReqLen = 6;

    if( varName.length() != 6 || varName[0] != 'v' )
    {
        throw HeliosException( "Invalid parameter to HeliosKWLEC370WR::SetVariable()");
    }

    try
    {
        _openModbus();

        // e.g. v01035=23.2
        // writeBuffer shall contain the ASCII representation of the parameter number in big endian
        uint16_t writeBuffer[ iReqLen ];
        char* writeData = (char*)writeBuffer;
        memset( writeBuffer, 0, iReqLen * 2 );
        sprintf( writeData, "%s=%.1f", varName.data(), value );
        _swapBytes( writeData, iReqLen * 2 );
        iRet = modbus_write_registers( myModbusPtr, 1, iReqLen, writeBuffer);

        if ( iRet == iReqLen )
        {
            PDEBUG("Written %d references.", iReqLen );
        }
        else
        {
            //fprintf (stderr, "Write %s failed: %s\n", (char*)writeData, modbus_strerror (errno));
            std::string errorMsg = "Write ";
            errorMsg += (char*)writeData;
            errorMsg += " failed: ";
            errorMsg += modbus_strerror (errno);
            throw HeliosException( errorMsg );
        }

        PDEBUG( "%s is set to %.1f", varName.data(), value );

        _closeModbus();
    }
    catch( HeliosException& e )
    {
        _closeModbus();
        throw e;
    }
}


// Bypass shall be closed if outdoor temperature is too low (<18 gradC) in order to prevent the room
// from cooling down. In spring to authumn when the weather is warm, the room may be overheated, so it is a good
// idea to open the bypass valve if:
//  - outdoor air temperature is above 18 grad Celsius
//  - room temperature > 23 grad AND room temperature is > outdoor temperatur

void HeliosKWLEC370WR::ControlBypassValve()
{
    myOutdoorTemp = _getAirTemperature( 1 );
    myRoomTemp = _getAirTemperature( 4 );
    myMinOutdoorTemp = _getBypassMinOutdoorTemp();

    if ( ( myOutdoorTemp > myMinOutdoorTemp ) && ( myOutdoorTemp < myRoomTemp ) && ( myRoomTemp > 23.0 ) )
    {
        // all three condition met
        // Room will be cooled by the cooler outdoor air
        _openBypass();
    }
    else if ( ( myOutdoorTemp < myMinOutdoorTemp ) || ( myOutdoorTemp > myRoomTemp ) || ( myRoomTemp < 23.0 ) )
    {
        // close the bypass if any of the above conditions meet
        _closeBypass();
    }
}


void HeliosKWLEC370WR::LogProcessData()
{
    LOGINFO( "%.1f, %.1f, %.1f", myOutdoorTemp.Value(), myRoomTemp.Value(), myMinOutdoorTemp.Value() );
}


std::string _getTimestamp()
{
    // formatting timestamp
    time_t rawtime;
    time (&rawtime);
    struct tm * timeinfo;
    timeinfo = localtime (&rawtime);
    char timestamp[ 256 ];
    strftime( timestamp, sizeof(timestamp), "%Y.%m.%d %H:%M:%S", timeinfo );

    return std::string( timestamp );
}


void HeliosKWLEC370WR::_openModbus()
{
    if ( myModbusPtr )
    {
        // already open
        return;
    }

    myModbusPtr = modbus_new_tcp_pi ( IPADDRESS, PORT );
    if ( myModbusPtr == NULL )
    {
        throw HeliosException( "Unable to create the libmodbus context" );
    }

    modbus_set_debug ( myModbusPtr, false);

    if ( modbus_connect( myModbusPtr ) == -1 )
    {
        modbus_free( myModbusPtr );
        myModbusPtr = NULL;
        std::string errMsg = "Connection failed: ";
        errMsg += modbus_strerror (errno);
        throw HeliosException( errMsg );
    }

    // Setting timeout
    uint32_t  sec, usec;
#ifdef DEBUG
    modbus_get_byte_timeout ( myModbusPtr, &sec, &usec);
    PDEBUG ("Get byte timeout: %d s, %d us", sec, usec);
#endif
    sec = (uint32_t) TIMEOUT; // 2 sec
    usec = (uint32_t) 0;
    modbus_set_response_timeout ( myModbusPtr, sec, usec);
    PDEBUG ("Set response timeout to %d sec, %d us", sec, usec);

    modbus_set_slave( myModbusPtr, SLAVEID );
}


void HeliosKWLEC370WR::_closeModbus()
{
    if ( myModbusPtr != NULL )
    {
        modbus_close( myModbusPtr );
        modbus_free( myModbusPtr );
    }

    myModbusPtr = NULL;
}


// 1: outdoor; 2: supply; 3: exhaust; 4: extract;
// 1: frisslevegő; 2: befúvás; 3: kifúvás; 4: elszívás

double HeliosKWLEC370WR::_getAirTemperature( int ventilNr )
{
    if( ventilNr < 1 || ventilNr > 4 )
    {
        throw HeliosException( "Invalid parameter to HeliosKWLEC370WR::_getAirTemperature()");
    }

    char varName[ 16 ];
    sprintf( varName, "v0010%d", ventilNr + 3 );
    std::string retVal = GetVariable( varName );

    double temperature = atof( retVal.c_str() );
    return temperature;
}


double HeliosKWLEC370WR::_getBypassMinOutdoorTemp()
{
    std::string retVal = GetVariable( "v01036" );

    double temperature = atof( retVal.c_str() );
    return temperature;
}


// By setting the bypass room temperature it is possible to open or close the bypass valve. If setting a higher room
// temperature value, wich will never exceeded, such as 40.0 grad celsius, the bypass closes. The bypass will be opened if
// the set bypass room temerature is below the actual room temperatur, e.g. 10 grad celsius.

void HeliosKWLEC370WR::_setBypassRoomTemperature( double roomTemp )
{
    if( roomTemp < 10.0 || roomTemp > 40.0 )
    {
        throw HeliosException( "Invalid parameter to HeliosKWLEC370WR::_setBypassRoomTemperature()");
    }

    SetVariable( "v01035", roomTemp );
}


void HeliosKWLEC370WR::_openBypass()
{
    _setBypassRoomTemperature( 10.0 );

    PDEBUG( "Bypass valve set to: OPEN" );

    if ( !myBypassState )
    {
        LogProcessData();
        printf( ">>>Bypass state changed to OPEN!\n");
    }

    myBypassState = true;
}


void HeliosKWLEC370WR::_closeBypass()
{
    _setBypassRoomTemperature( 40.0 );

    PDEBUG( "Bypass valve set to: CLOSED" );

    if ( myBypassState )
    {
        LogProcessData();
        printf( ">>>Bypass state changed to CLOSED!\n");
    }

    myBypassState = false;
}


void HeliosKWLEC370WR::_swapBytes( char* data, int len )
{
    if ( len < 2 )
    {
        throw HeliosException( "Invalid parameter to HeliosKWLEC370WR::_swapBytes()");
    }

    char temp;
    for ( int i = 0; i < len - 1; i += 2 )
    {
        temp = data[ i ];
        data[ i ] = data[ i + 1 ];
        data[ i + 1 ] = temp;
    }
}
