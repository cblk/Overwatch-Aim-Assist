﻿/*
 *Overwatch Aim Assitance
 *Copyright (C) 2016  Juan Xuereb
 *
 *This program is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.

 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Capture.h"

Capture::Capture(char *windowName)
{
	_WindowName = windowName;
}

Capture::~Capture()
{
}

bool Capture::isWindowRunning()
{
	HWND hwnd = FindWindowA(0, _WindowName);
	return (hwnd != NULL);
}

int Capture::getWidth() {
	HWND hwnd = FindWindowA(0, _WindowName);
	if (hwnd == NULL) return 0;

	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	return rekt.right - rekt.left;
}

int Capture::getHeight(){
	HWND hwnd = FindWindowA(0, _WindowName);
	if (hwnd == NULL) return 0;

	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	return rekt.bottom - rekt.top;
}

void Capture::switchToWindow(){
	HWND hwnd = FindWindowA(0, _WindowName);
	if(hwnd!=NULL) SwitchToThisWindow(hwnd, false);
}


/* Captures a screenshot and saves it into screeny which is passed by reference
   Return trype T/F indicates whether method was successful or not. */
bool Capture::screenshotGDI(Screenshot &screeny)
{
	HWND hwnd = FindWindowA(0, _WindowName);
	if (hwnd == NULL) return false;

	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	int width = rekt.right - rekt.left;
	int height = rekt.bottom - rekt.top;

	//GDI screen capture
	HDC hdcShot = CreateCompatibleDC(0);
	HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);

	SelectObject(hdcShot, hBmp);

	BitBlt(hdcShot, 0, 0, width, height, GetDC(0), rekt.left, rekt.top, SRCCOPY);

	BITMAP bmp;
	BITMAPINFO bmpInfo;

	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		cout << "Failed to capture screenshot @GetObject()" << endl;
		ReleaseDC(hwnd, hdcShot);
		DeleteObject(hBmp);
		DeleteDC(hdcShot);
		return false;
	}

	if (bmp.bmBitsPixel != 32 || bmp.bmPlanes != 1)
	{
		cout << "ERROR: BitsPerPixels=" << bmp.bmBitsPixel << " Planes=" << bmp.bmPlanes << endl;
		ReleaseDC(hwnd, hdcShot);
		DeleteObject(hBmp);
		DeleteDC(hdcShot);
		return false;
	}
	//

	//Bitmap information
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
	bmpInfo.bmiHeader.biHeight = bmp.bmHeight;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bmp.bmBitsPixel;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = 0;
	//

	screeny.FreeMemory();

	int pixNo = bmp.bmWidth * bmp.bmHeight;
	screeny.pixels = new RGBQUAD[pixNo];
	if (!screeny.pixels)
	{
		cout << "ERROR: Allocating RGBQUAD[" << pixNo << "]" << endl;
		ReleaseDC(hwnd, hdcShot);
		DeleteObject(hBmp);
		DeleteDC(hdcShot);
		return false;
	}

	if (!GetDIBits(hdcShot, hBmp, 0, bmp.bmHeight, screeny.pixels, &bmpInfo, DIB_RGB_COLORS))
	{
		cout << "ERROR: GetDIBits!" << endl;
		screeny.FreeMemory();
		ReleaseDC(hwnd, hdcShot);
		DeleteObject(hBmp);
		DeleteDC(hdcShot);
		return false;
	}

	screeny.width = bmp.bmWidth;
	screeny.height = bmp.bmHeight;
	screeny.length = pixNo;

	if (screeny.isScreenyBlack())
	{
		cout << "ERROR: Screenshot is black! Please run in borderless windowed mode!!!" << endl;
		ReleaseDC(hwnd, hdcShot);
		DeleteObject(hBmp);
		DeleteDC(hdcShot);
		return false;
	}

	ReleaseDC(hwnd, hdcShot);
	DeleteObject(hBmp);
	DeleteDC(hdcShot);

	return true;
}


void Capture::waitTillNextFrame(Screenshot &currentFrame)
{
	Midline prevFrameMidline(currentFrame);
	bool newFrameLoaded = false;
	while (!newFrameLoaded) //Wait till new frame loads!
	{
		screenshotGDI(currentFrame);

		if (currentFrame != prevFrameMidline)
			newFrameLoaded = true;
		else
			Sleep(1);
	}
}