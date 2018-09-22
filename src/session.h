#pragma once
#include "ofMain.h"
#include "tracks/base.h"

namespace ofxLiveSet
{
class session
{
public:
	session()
	{
		ofAddListener(ofEvents().draw, this, &session::onDraw, OF_EVENT_ORDER_AFTER_APP);
		ofAddListener(ofEvents().update, this, &session::onUpdate, OF_EVENT_ORDER_AFTER_APP);
	}
	void update()
	{
		for (auto track : _tracks)
		{
			track->update();
		}
	}
	void draw()
	{
		for (auto track : _tracks)
		{
            track->draw();
		}
	}
	void onUpdate(ofEventArgs &e)
	{
		update();
	}

	void onDraw(ofEventArgs &e)
	{
		draw();
	}

	void start()
	{
	}
	void pause()
	{
	}
	void stop()
	{
		for (auto track : _tracks)
		{
			track->stop();
		}
	}
	void addTrack(track::base *track)
	{
		_tracks.push_back(track);
	}
	std::vector<track::base *> _tracks;
	ofParameterGroup _parameters;
	ofParameter<std::string> _name;
};
}; // namespace ofxLiveSet
