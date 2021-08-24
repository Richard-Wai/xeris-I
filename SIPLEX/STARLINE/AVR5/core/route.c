/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All rights reserved.
 */

/*
 *  XERIS CORE
 *
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  SIPLEX/STARLINE (AVR5)
 *
 *  Siplex Core Shot Router
 *
 */

#include <xeris.h>

#include <siplex/siplex.h>

volatile struct shotqueue * siplex_route ( uint8_t intent, uint16_t permit )
{
     /* Major to-do here in the future   */
     /* for now we just "blindly" return */
     /* all the relevent mainline queues */

     switch ( intent )
     {
          case SHOT_CMD:
               return ( &queues[INIT_Q_CMD] );
               break;

          case SHOT_TRA:
               return ( &queues[INIT_Q_TRA] );
               break;
               
          case SHOT_CYC:
               return ( &queues[INIT_Q_CYC] );
               break;

          default:
               break;
     }

     return ( NULL );
}
