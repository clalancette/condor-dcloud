##**************************************************************
##
## Copyright (C) 1990-2011, Condor Team, Computer Sciences Department,
## University of Wisconsin-Madison, WI.
## 
## Licensed under the Apache License, Version 2.0 (the "License"); you
## may not use this file except in compliance with the License.  You may
## obtain a copy of the License at
## 
##    http://www.apache.org/licenses/LICENSE-2.0
## 
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
##**************************************************************

MACRO( GET_TIMEDATE _TIMEDATE )
    if(NOT WINDOWS)
        execute_process( COMMAND date "+%Y%m%d_%H%M%S" OUTPUT_VARIABLE ${_TIMEDATE} OUTPUT_STRIP_TRAILING_WHITESPACE )
    else()
        execute_process( COMMAND date "/T" OUTPUT_VARIABLE _date OUTPUT_STRIP_TRAILING_WHITESPACE )
        execute_process( COMMAND time "/T" OUTPUT_VARIABLE _time OUTPUT_STRIP_TRAILING_WHITESPACE )
        if( ${_date} AND ${_time} )
            string(REGEX MATCH "([0-9]+)/([0-9]+)/([0-9]+)" DATE_MATCH "${_date}" )
            set( _date "${CMAKE_MATCH_3}${CMAKE_MATCH_1}${CMAKE_MATCH_2}" )
            string( REGEX MATCH "([0-9]+):([0-9]+) ([AP]M)" TIME_MATCH "${_time}" )
            if( ${CMAKE_MATCH_3} STREQUAL "PM" )
                MATH(EXPR HOUR "${CMAKE_MATCH_1} + 12")
            else()
                set( HOUR "${CMAKE_MATCH_1}" )
            endif()
            set( _time "${HOUR}${CMAKE_MATCH_2}" )
            set( ${_TIMEDATE} "${_date}_${HOUR}${CMAKE_MATCH_2}" )
        else()
            set( ${_TIMEDATE} "" )
        endif()
    endif()
ENDMACRO( GET_TIMEDATE )
