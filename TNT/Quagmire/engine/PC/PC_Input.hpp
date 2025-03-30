#ifndef PCINPUT_HPP
#define PCINPUT_HPP

#include "DInput.h"
#include "Q_Input.hpp"
#include "x_Types.hpp"

err		INPUT_InitModule       ( HWND hWnd );
void	INPUT_KillModule       ( void );
void	INPUT_CheckDevices     ( void );

#endif