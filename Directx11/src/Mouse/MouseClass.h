#pragma once
#include "MouseEvent.h"

class MouseClass
{
public:
	void OnLeftPressed(int x, int y);
	void OnLeftReleased(int x, int y);
	void OnRightPressed(int x, int y);
	void OnRightReleased(int x, int y);
	void OnMiddlePressed(int x, int y);
	void OnMiddleReleased(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);
	void OnMouseMove(int x, int y);
	void EndFrame();
	void OnMouseMoveRaw(int x, int y);

	bool IsLeftDown();
	bool IsMiddleDown();
	bool IsRightDown();

	int GetPosX();
	int GetPosY();
	int GetDeltaX() const;
	int GetDeltaY() const;
	void MouseMoveRaw(int newX, int newY);
	MousePoint GetPos();
	//void OnMouseMoveRaw(int newX, int newY);
	bool EventBufferIsEmpty();
	MouseEvent ReadEvent();

private:
	std::queue<MouseEvent> eventBuffer;
	bool leftIsDown = false;
	bool rightIsDown = false;
	bool mbuttonDown = false;
	int deltaX = 0;
	int deltaY = 0;
	int oldx = 0;
	int oldy = 0;
	int x = 0;
	int y = 0;
};