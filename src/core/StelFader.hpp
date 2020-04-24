/*
 * ShiraPlayer(TM)
 * Copyright (C) 2005 Fabien Chereau
 * Copyright (C) 2011 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */

#ifndef _STELFADER_HPP_
#define _STELFADER_HPP_

//! @class StelFader
//! Manages a (usually smooth) transition between two states (typically ON/OFF) in function of a counter
//! It used for various purpose like smooth transitions between 
class StelFader
{
public:
	// Create and initialise
	StelFader(bool initialState, float minimumValue=0.f, float maximumValue=1.f) : state(initialState), minValue(minimumValue), maxValue(maximumValue) {;}
	virtual ~StelFader() {;}
	// Increments the internal counter of deltaTime ticks
	virtual void update(int deltaTicks) = 0;
	// Gets current switch state
	virtual float getInterstate(void) const = 0;
	virtual float getInterstatePercentage(void) const = 0;
	// Switchors can be used just as bools
	virtual StelFader& operator=(bool s) = 0;
	bool operator==(bool s) const {return state==s;}
	operator bool() const {return state;}
	virtual void setDuration(int _duration) {;}
	virtual float getDuration(void) = 0;
	virtual void setMinValue(float _min) {minValue = _min;}
	virtual void setMaxValue(float _max) {maxValue = _max;}
	float getMinValue(void) {return minValue;}
	float getMaxValue(void) {return maxValue;}    
protected:
	bool state;
	float minValue, maxValue;
};

//! @class BooleanFader
//! Implementation of StelFader which behaves like a normal boolean, i.e. no transition between on and off.
class BooleanFader : public StelFader
{
public:
	// Create and initialise
	BooleanFader(bool initialState=false, float minimumValue=0.f, float maximumValue=1.f) : StelFader(initialState, minimumValue, maximumValue) {;}
	~BooleanFader() {;}
	// Increments the internal counter of deltaTime ticks
	void update(int deltaTicks) {;}
	// Gets current switch state
	float getInterstate(void) const {return state ? maxValue : minValue;}
	float getInterstatePercentage(void) const {return state ? 100.f : 0.f;}
	// Switchors can be used just as bools
	StelFader& operator=(bool s) {state=s; return *this;}
	virtual float getDuration(void) {return 0.f;}
protected:
};

//! @class LinearFader
//! Implementation of StelFader which implements a linear transition.
//! Please note that state is updated instantaneously, so if you need to draw something fading in
//! and out, you need to check the interstate value (!=0) to know to draw when on AND during transitions.
class LinearFader : public StelFader
{
public:
	// Create and initialise to default
	LinearFader(int _duration=1000, float minimumValue=0.f, float maximumValue=1.f, bool initialState=false) 
		: StelFader(initialState, minimumValue, maximumValue)
	{
		isTransiting = false;
		duration = _duration;
        interstate = state ? maxValue : minValue;
	}
	
	~LinearFader() {;}
	
	// Increments the internal counter of deltaTime ticks
	void update(int deltaTicks)
	{
		if (!isTransiting) return; // We are not in transition
        counter+=deltaTicks;
		if (counter>=duration)
		{
			// Transition is over
			isTransiting = false;
			interstate = targetValue;
			// state = (targetValue==maxValue) ? true : false;
		}
		else
        {
            interstate = startValue + (targetValue - startValue) * counter/duration;
		}
	}
	
	// Get current switch state
	float getInterstate(void) const { return interstate;}
	float getInterstatePercentage(void) const {return 100.f * (interstate-minValue)/(maxValue-minValue);}
	
	// StelFaders can be used just as bools
	StelFader& operator=(bool s)
	{

		if(isTransiting) {
			// if same end state, no changes
			if(s == state) return *this;
			
			// otherwise need to reverse course
			state = s;
			counter = duration - counter;
			float temp = startValue;
			startValue = targetValue;
			targetValue = temp;
			
		} else {

			if(state == s) return *this;  // no change

			// set up and begin transit
			state = s;
			startValue = s ? minValue : maxValue;
			targetValue = s ? maxValue : minValue;
			counter=0;
			isTransiting = true;
		}
		return *this;
	}
	
	void setDuration(int _duration) {duration = _duration;}
	virtual float getDuration(void) {return duration;}
	void setMaxValue(float _max) {
		if(interstate >=  maxValue) interstate =_max;
		maxValue = _max;
	}
    void reset(){ interstate = 0 ; state = false;}
protected:
	bool isTransiting;
	int duration;
	float startValue, targetValue;
	int counter;
    float interstate;
};


// Please note that state is updated instantaneously, so if you need to draw something fading in
// and out, you need to check the interstate value (!=0) to know to draw when on AND during transitions
class ParabolicFader : public StelFader
{
public:
	// Create and initialise to default
	ParabolicFader(int _duration=1000, float minimumValue=0.f, float maximumValue=1.f, bool initialState=false) 
		: StelFader(initialState, minimumValue, maximumValue)
	{
		isTransiting = false;
		duration = _duration;
		interstate = state ? maxValue : minValue;
	}
	
	~ParabolicFader() {;}
	
	// Increments the internal counter of deltaTime ticks
	void update(int deltaTicks)
	{
		if (!isTransiting) return; // We are not in transition
		counter+=deltaTicks;
		if (counter>=duration)
		{
			// Transition is over
			isTransiting = false;
			interstate = targetValue;
			// state = (targetValue==maxValue) ? true : false;
		}
		else
		{
			interstate = startValue + (targetValue - startValue) * counter/duration;
			interstate *= interstate;
		}

		// printf("Counter %d  interstate %f\n", counter, interstate);
	}
	
	// Get current switch state
	float getInterstate(void) const { return interstate;}
	float getInterstatePercentage(void) const {return 100.f * (interstate-minValue)/(maxValue-minValue);}
	
	// StelFaders can be used just as bools
	StelFader& operator=(bool s)
	{

		if(isTransiting) {
			// if same end state, no changes
			if(s == state) return *this;
			
			// otherwise need to reverse course
			state = s;
			counter = duration - counter;
			float temp = startValue;
			startValue = targetValue;
			targetValue = temp;
			
		} else {

			if(state == s) return *this;  // no change

			// set up and begin transit
			state = s;
			startValue = s ? minValue : maxValue;
			targetValue = s ? maxValue : minValue;
			counter=0;
			isTransiting = true;
		}
		return *this;
	}
	
	void setDuration(int _duration) {duration = _duration;}
	virtual float getDuration(void) {return duration;}
protected:
	bool isTransiting;
	int duration;
	float startValue, targetValue;
	int counter;
	float interstate;
};

#endif // _STELFADER_HPP_
