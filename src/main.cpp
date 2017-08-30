//*****************************************************************************
//$Workfile: main.cpp
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


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sysio/delay.h>

#include "version-git.h"
#include "helios.h"


/* constants ================================================================ */
#define AUTHORS "Peter Nagy"
#define WEBSITE "http://www.perit.hu"

void vFailureExit (bool bHelp, const char *format, ...);
void vSigIntHandler (int sig);
void vHello (void);

#define vIoErrorExit(fmt,...) vFailureExit(false,fmt,##__VA_ARGS__)

static char * progname;
static HeliosKWLEC370WR* pHelios = NULL;

#pragma GCC diagnostic ignored "-Wunused-parameter"

int main( int argc, char **argv )
{
    setbuf( stdout, NULL );

    progname = argv[0];

    // vSigIntHandler() handling of CTRL+C
    signal (SIGINT, vSigIntHandler);

    vHello();

    pHelios = new HeliosKWLEC370WR();
    if ( pHelios == NULL )
    {
        vIoErrorExit ("Unable to create the HeliosKWLEC370WR object");
    }

#ifdef DEBUG
        int cycleTime = 1000; // 1 sec
#else
        int cycleTime = 60 * 1000; // 1 min
#endif
    int cycleCounter = 100;

    printf( "Timestamp, Outdoor temp, Room temp, Min outdoor temp\n" );

    while ( 1 )
    {
        delay_ms( cycleTime );

        cycleCounter++;

        // Actions in each cycle
        try
        {
            //pHelios->ModulTest();
            //return 0;
            pHelios->ControlBypassValve();
        }
        catch( HeliosException& e )
        {
            LOGERR( "%s", e.what() );
        }
        catch(...)
        {
            vIoErrorExit ("Unhandled exception caught");
        }

        // Actions in all 5th cycle
        if ( cycleCounter >= 5 )
        {
            cycleCounter = 0;
            pHelios->LogProcessData();
        }
    }

    vSigIntHandler (SIGTERM);

    return 0;
}


// -----------------------------------------------------------------------------
void vHello (void)
{
  printf ("\n%s %s - Helios bypass control through modbus interface\n",
          basename (progname), VERSION_SHORT);
  printf ("Copyright (c) 2017 %s\n"
          "This software is governed by the GNU General Public License <http://www.gnu.org/licenses/>\n\n"
          , AUTHORS);
}


// -----------------------------------------------------------------------------
void vFailureExit (bool bHelp, const char *format, ...) {
  va_list va;

  if ( pHelios != NULL )
  {
      delete pHelios;
      pHelios = NULL;
  }

  va_start (va, format);
  fprintf (stderr, "%s: ", progname);
  vfprintf (stderr, format, va);
  if (bHelp) {
    fprintf (stderr, " ! Try -h for help.\n");
  }
  else {
    fprintf (stderr, ".\n");
  }
  va_end (va);
  fflush (stderr);
  exit (EXIT_FAILURE);
}


// -----------------------------------------------------------------------------
void vSigIntHandler (int sig)
{
    if ( pHelios != NULL )
    {
        delete pHelios;
        pHelios = NULL;
    }

    if (sig == SIGINT)
    {
        printf ("\neverything was closed.\nHave a nice day !\n");
    }
    else
    {
        putchar ('\n');
    }
    fflush (stdout);
    exit (EXIT_SUCCESS);
}
