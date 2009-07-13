/***************************************************************************
 *   Copyright (C) 2005 by Alexey Obukhov                                  *
 *   obukhoff@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
	Modification history:
	oct-2006 - Denis Kovalev - compilation issues
*/

#pragma once

#include <time.h>
#include <string>

class opcError
{
public:
	time_t	stamp;
	std::string message;

	opcError() 
	{
		time(&stamp);
		message = "Unknown error";
	}

	opcError(std::string msg) 
	{
		message = msg;
		time(&stamp);
	}

	opcError( const char *fmt, ... )
	{
		static char buffer[2048] = {0};

		va_list args;
		va_start(args, fmt);
		_vsnprintf (buffer, 2048, fmt, args);
		va_end(args);

		message = buffer;
		time(&stamp);
	}

	inline const char *GetMessage() 
	{
		return message.c_str();
	}
};
