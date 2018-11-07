#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "session.h"

namespace ofxLiveSet{
namespace clip{
class base{
public:
    base(std::string name = ""){
        _active.addListener(this, &base::onActiveChange);
        _parameters.setName("clip");
        _name.set("name", name);
        _active.set("active", false);
        
//        _parameters.add(_name);
        _parameters.add(_active);
        
 
	}
    virtual void setup(){
        //        ofxPanel::setDefaultWidth(200);
        ofxPanel::setDefaultFillColor(ofColor::blue);
        _gui.setup(_parameters);
        _gui.setHeaderBackgroundColor(ofColor::blue);
        _gui.setUseTTF(true);
        _gui.setFillColor(ofColor::red);
    }
	virtual void update(){}
	virtual void draw(){}
	virtual void start(){
        _active = true;
	}
	virtual void pause(){}
	virtual void stop(){
        _active = false;
	}
    void toggle(){
        _active = !_active;
    }
    
    void onActiveChange(bool & value) {
        bool valueToBeNotified = true;
        if(value){
            ofNotifyEvent(_started, valueToBeNotified, this);
        } else {
            ofNotifyEvent(_stopped, valueToBeNotified, this);
        }
    }
	ofParameterGroup _parameters;
	ofParameter<std::string> _name;
	ofParameter<bool> _active;

     ofEvent<bool> _started;
     ofEvent<bool> _finished;
     ofEvent<bool> _stopped;
    
    ofxPanel _gui;
};
}; // namespace clip
}; // namespace ofxLiveSet
