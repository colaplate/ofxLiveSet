#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxMidiMapper.h"
#include "ofxKeyMapper.h"
#include "ofxOscMapper.h"
//#include "mqttSynchroniser.h"
#include "./tracks/base.h"
#include "./gui/infoPanel.h"

namespace ofxLiveSet {
class session {
public:
	session()
	{
		ofAddListener(ofEvents().update, this, &session::onUpdate, OF_EVENT_ORDER_AFTER_APP);
        ofAddListener(ofEvents().draw, this, &session::onDraw, OF_EVENT_ORDER_BEFORE_APP);
        ofAddListener(ofEvents().exit, this, &session::onExit, OF_EVENT_ORDER_AFTER_APP);

//        ofAddListener(ofEvents().keyPressed, this, &session::onKeyPressed, OF_EVENT_ORDER_AFTER_APP);

        _defaultMapping.set("defaultMapping", true);
	}
    void setup() {
        for(auto track : _tracks){
            track->setup();
        }
        
        
        _parameters.setName("session");
        _name.set("name", "session");
        _active.set("active", false);
        _timestampString.set("time", "0");
        _mute.set("mute", false);
        _gain.set("gain", 1, 0, 1); //TODO: does make gain sense for a session with audio, dmx and midi tracks?

//        _parameters.add(_name);
        _parameters.add(_active);
        _parameters.add(_timestampString);
        _parameters.add(_mute);
        _parameters.add(_gain);

        _active.addListener(this, &session::onActive);

        
        auto scenes = 0;
        for(auto track : _tracks){
            scenes = std::max(scenes, (int)(track->_clips.size()));
        }
        _sceneTriggers.resize(scenes);
        _sceneInformation.resize(scenes);
        auto i = 0;
        for(auto sceneTrigger : _sceneTriggers){
            sceneTrigger.set("scene "+ofToString(i++), false);
            _parameters.add(sceneTrigger);
            sceneTrigger.addListener(this, &session::onSceneTrigger);
        }

        
        for(auto track : _tracks){
            for(auto clip : track->_clips){
                ofAddListener(clip->_started, this, &session::onClipStarted);
            }
        }
        
        _focusedTrack.set("focusedTrack", 0);
        _focusedClip.set("focusedClip", 0);

        for(auto track : _tracks){
            for(auto clip : track->_clips){
                _midiMapper.addParameters(clip->_parameters, true);
                _keyMapper.addParameters(clip->_parameters);
            }
        }

                
        if(ofFile::doesFileExist("mapping.midi.json")){
            _midiMapper.loadMapping(ofToDataPath("mapping.midi.json"));
        }
        
        if(ofFile::doesFileExist("mapping.key.json")){
            _keyMapper.loadMapping(ofToDataPath("mapping.key.json"));
        }
    }
    void setupGui(){
        ofxPanel::setDefaultFillColor(ofColor::green);
        _midiMapper.getParameters().setName("midi mapper");
        _midiMapperPanel.setup(_midiMapper.getParameters());
        _midiMapperPanel.setPosition(0, ofGetHeight()/2);
        _midiMapperPanel.setHeaderBackgroundColor(ofColor::green);
        _keyMapper.getParameters().setName("key mapper");
        _keyMapperPanel.setup(_keyMapper.getParameters());
        _keyMapperPanel.setPosition(_midiMapperPanel.getPosition().x, _midiMapperPanel.getPosition().y + _midiMapperPanel.getHeight());
        _keyMapperPanel.setHeaderBackgroundColor(ofColor::green);
        _oscMapper.getParameters().setName("osc mapper");
        _oscMapperPanel.setup(_oscMapper.getParameters());
        _oscMapperPanel.setPosition(_keyMapperPanel.getPosition().x, _keyMapperPanel.getPosition().y + _keyMapperPanel.getHeight());
        _oscMapperPanel.setHeaderBackgroundColor(ofColor::green);

        ofxPanel::setDefaultFillColor(ofColor::green);
//        _scenesPanel.setDefaultBorderColor(ofColor::green);
        _scenesPanel.setup(_parameters);
        _scenesPanel.setHeaderBackgroundColor(ofColor::green);
        _scenesPanel.setPosition(ofGetWidth() - _scenesPanel.getWidth(), 0);
        _scenesPanel.setUseTTF(true);

        auto x = 0;
        auto y = 0;
        for(auto track : _tracks){
            track->setupGui();
            track->_gui.setPosition(x, y);
            track->_outputGui.setPosition(_midiMapperPanel.getPosition().x + _midiMapperPanel.getWidth(), ofGetHeight()/2);

            x += track->_gui.getWidth() + 1; //TODO: get border width

            for(auto clip : track->_clips){
                clip->_gui.setPosition(track->_outputGui.getPosition().x + track->_outputGui.getWidth(), ofGetHeight()/2);
            }
        }
        _infoPanel.setup();
    }
    void openMidiMapperInPort(int index){
        _midiMapper.openMidiPort(index);
    }
    void openOscInPort(int port){
        _oscReceiver.setup(port);
    }
	void update(){
        if(_active){
            _timestamp = ofGetElapsedTimeMillis();
            _timestampString = ofToString((int)((_timestamp - _startedTimestamp)/1000));
            for (auto track : _tracks)
            {
                track->update();
            }
        }
        auto i = 0;
        for (auto track : _tracks)
        {
            track->_focused = _focusedTrack == i++;
            track->update();
        }

//    TODO: add more commands, check oscparamsync with multiple parameter groups having the same name, 
        while(_oscReceiver.hasWaitingMessages()){
            ofxOscMessage m;
            _oscReceiver.getNextMessage(m);

            if(m.getAddress() == "/ofxSession/scene/trigger"){
                auto index = m.getArgAsInt(0);
                triggerScene(index);
            }
            else if(m.getAddress() == "/ofxSession/stop"){
                stop();
            }
            else if(m.getAddress() == "/ofxSession/track/stop"){
                auto index = m.getArgAsInt(0);
                if(index < _tracks.size()){
                    _tracks[index]->stop();
                }
            }
            else if(m.getAddress() == "/ofxSession/clip/toggle"){
                auto trackIndex = m.getArgAsInt(0);
                auto clipIndex = m.getArgAsInt(1);

                if(trackIndex < _tracks.size()){
                    auto track = _tracks[trackIndex];
                    if(clipIndex < track->_clips.size()){
                        auto clip = track->_clips[clipIndex];
                        clip->toggle();
                    }
                }
            }
        }
//        _mqttSynchroniser.update();
	}
	void draw(){
		for (auto track : _tracks){
            track->draw();
		}
	}
    void drawGui(){
        for (auto track : _tracks){
            track->drawGui();
		}
        _scenesPanel.setPosition(ofGetWidth() - _scenesPanel.getWidth(),0); //TODO: only set position on resize

        _scenesPanel.draw();
        _midiMapperPanel.draw();
        _keyMapperPanel.draw();
        _oscMapperPanel.draw();

        if(_focusedTrack < _tracks.size() && _focusedClip < _tracks[_focusedTrack]->_clips.size()){
            _tracks[_focusedTrack]->_clips[_focusedClip]->_gui.draw();
        }
        _infoPanel.draw(0, ofGetHeight() - 100, ofGetWidth()/4, 100);
    }
    void exit(){
        if(ofFile::doesFileExist("mapping.midi.json")){
            ofFile::moveFromTo("mapping.midi.json", ofGetTimestampString()+"_mapping.midi.json");
        }
        _midiMapper.saveMapping(ofToDataPath("mapping.midi.json"));
        
        if(ofFile::doesFileExist("mapping.key.json")){
            ofFile::moveFromTo("mapping.key.json", ofGetTimestampString()+"_mapping.key.json");
        }
        _keyMapper.saveMapping(ofToDataPath("mapping.key.json"));
    }
    
	void onUpdate(ofEventArgs &e){
		update();
	}

	void onDraw(ofEventArgs &e){
		draw();
	}
    void onExit(ofEventArgs &e)
    {
        exit();
    }
    void onKeyPressed(int key){
        _keyMapper.keyPressed(key);
        if(_defaultMapping){
            switch(key){
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    triggerScene(key-48);
                    break;

                case OF_KEY_LEFT: {
                    _focusedTrack = std::max(0, _focusedTrack-1);
                    break;
                }
                case OF_KEY_RIGHT: {
                    _focusedTrack = std::min((int)(_tracks.size())-1, (int)((_focusedTrack+1) % _tracks.size()));
                    break;
                }
                case OF_KEY_UP: {
                    _focusedClip = std::max(0, _focusedClip-1);
                    break;
                }
                case OF_KEY_DOWN: {
                    _focusedClip = std::min((int)(_tracks[_focusedTrack]->_clips.size())-1, _focusedClip+1);
                    break;
                }
                case OF_KEY_RETURN: {
                    auto clip = getClip(_focusedTrack, _focusedClip);
                    if(clip != nullptr){
                        clip->toggle();
                    }
                    break;
                }
                case ' ': {
                    toggle();
                }
            }
        }
    }
    void toggle(){
        _active = !_active;
    }
	void start()
	{
        _startedTimestamp = ofGetElapsedTimeMillis();
        for (auto track : _tracks)
        {
            track->start();
        }
	}
	void pause()
	{
	}
	void stop(){
        _timestampString = "";
		for (auto track : _tracks)
		{
			track->stop();
		}
	}
    track::base * addTrack(track::base *track)
	{
		_tracks.push_back(track);
        return track;
	}
    
    void triggerScene(int index){
        for (auto track : _tracks){
            track->stop();
            track->trigger(index);
        }
        if(index < _sceneInformation.size()){
            _infoPanel.setInfo(_sceneInformation[index]);
        }
    }
    
    clip::base* getClip(int track, int index){
        if(track >= _tracks.size()){ return nullptr; }
        if(index >= _tracks[track]->_clips.size()){ return nullptr; }
        return _tracks[track]->_clips[index];
    }
    
    void renameScene(int index, std::string name){
        if(index >= _sceneTriggers.size()){
            ofLogError("ofxLiveSet::session") << "cannot rename scene. index (" << index << ") out of bounds. ("<<_sceneTriggers.size()<<")";
            return;
        }
        _sceneTriggers[index].setName(name);
    }
    void onSceneTrigger(const void* sender, bool & value) {
        if(!value){ return; }
        auto i = 0;
        for(auto sceneTrigger : _sceneTriggers) {
            if(sceneTrigger){
                sceneTrigger = false;
                triggerScene(i);
            }
            i++;
        }
    }
    
    void onActive(bool & value){
        if(value){
            start();
        }else{
            stop();
        }
    }
    void onClipStarted(const void* sender, bool & value) {
        auto clip = (clip::base *) (sender);
        auto nullClip = dynamic_cast<ofxLiveSet::clip::nullClip *>(clip);
        if (nullClip != nullptr){return;}
        if(!_active){
            _active = true;
        }
        auto trackIndex = 0;
        auto clipIndex = 0;
        for(auto track : _tracks){
            for(auto clipToCompareWith : track->_clips){
                if(clip == clipToCompareWith){
                    _focusedTrack = trackIndex;
                    _focusedClip = clipIndex;
                }
                clipIndex++;
            }
            clipIndex = 0;
            trackIndex++;
        }
    }

	std::vector<track::base *> _tracks;
	ofParameterGroup _parameters;
	ofParameter<std::string> _name;
    ofParameter<bool> _active;
    ofParameter<std::string> _timestampString;
    ofParameter<bool> _mute;
    ofParameter<float> _gain;

    std::vector<ofParameter<bool>> _sceneTriggers;
    ofParameter<int> _focusedTrack;
    ofParameter<int> _focusedClip;

    ofxPanel _scenesPanel;
    ofxPanel _midiMapperPanel;
    ofxPanel _keyMapperPanel;
    ofxPanel _oscMapperPanel;

    ofxMidiMapper _midiMapper;
    ofxKeyMapper _keyMapper;
    ofxOscMapper _oscMapper;
    ofxOscReceiver _oscReceiver;

    u_int64_t _timestamp;
    u_int64_t _startedTimestamp;
    
//    mqttSynchroniser _mqttSynchroniser;
        
    gui::infoPanel _infoPanel;
    std::vector<ofxLiveSet::information> _sceneInformation;
    
    ofParameter<bool> _defaultMapping;

};
}; // namespace ofxLiveSet
