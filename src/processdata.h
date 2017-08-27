#ifndef PROCESSDATA_H
#define PROCESSDATA_H

//*****************************************************************************
//$Workfile: processdata.h
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
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//*****************************************************************************


//*** INCLUDES ****************************************************************


//*** DEFINES AND CONSTANTS ***************************************************

//*** TYPES AND CLASSES *******************************************************

class ProcessData
{
    //------- Enums, Typedefs -------

    //------- Attribute -------------
    protected:
        double          myValue = 0.0;
        double          myHysteresis = 0.0;

    //------- Services --------------
    public:
        // Constructor
                        ProcessData( double value = 0.0 )
                        {
                            myValue = value;
                        };

                        ProcessData( const ProcessData& other )
                        {
                            *this = other;
                        };

        virtual void    SetHysteresis( double value )
                        {
                            myHysteresis = value;
                        }

        virtual void operator=( const ProcessData& other )
                        {
                            myValue = other.myValue;
                            myHysteresis = other.myHysteresis;
                        }

        virtual bool operator>( const double value )
                        {
                            return ( myValue > value + myHysteresis );
                        }

        virtual bool operator>( const ProcessData other )
                        {
                            return *this > other.myValue;
                        }

        virtual bool operator<( const double value )
                        {
                            return ( myValue < value - myHysteresis );
                        }

        virtual bool operator<( const ProcessData other )
                        {
                            return *this < other.myValue;
                        }

        virtual double  Value()
                        {
                            return myValue;
                        }
};


#endif // PROCESSDATA_H
