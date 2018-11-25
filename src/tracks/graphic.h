#pragma once
#include "ofMain.h"
#include "base.h"
#include "../clips/graphic.h"

namespace ofxLiveSet
{
namespace track
{
class graphic : public base
{
public:
    graphic(std::string name = "") : base(name){
        _xPosition.set("x", 0, 0, ofGetWidth());
        _yPosition.set("y", 0, 0, ofGetHeight());
        _width.set("width", ofGetWidth(), 0, ofGetWidth());
        _height.set("height", ofGetHeight(), 0, ofGetHeight());

        _width.addListener(this, &graphic::onWidthChange);
        _height.addListener(this, &graphic::onHeightChange);

        _outputParameters.add(_xPosition);
        _outputParameters.add(_yPosition);
        _outputParameters.add(_width);
        _outputParameters.add(_height);
        _fbo.allocate(_width, _height);
	}
    virtual void draw(){
        base::draw();
        auto clip = ((ofxLiveSet::clip::graphic *)(_clip));
        if(clip == nullptr) return;

        if(!_mute && clip != nullptr && clip->isFrameNew()) {
            _fbo.begin();
            ofClear(255,0);
            // TODO: apply gain
            // ofSetColor(255, _gain * 255);
            clip->_fbo.draw(0,0);
            _fbo.end();
        }
        _fbo.draw(_xPosition, _yPosition);
    }
    void onWidthChange(float & value){
        _fbo.allocate(_width, _height);
        for(auto clip : _clips){
            auto graphicClip = dynamic_cast<ofxLiveSet::clip::graphic *>(clip);
            if (graphicClip != nullptr) {
                graphicClip->_fbo.allocate(_width, _height);
            }
        }
    }
    void onHeightChange(float & value){
        _fbo.allocate(_width, _height);
        _fbo.begin();
        ofClear(255,0);
        _fbo.end();
        for(auto clip : _clips){
            auto graphicClip = dynamic_cast<ofxLiveSet::clip::graphic *>(clip);
            if (graphicClip != nullptr) {
                graphicClip->_width = _width;
                graphicClip->_height = _height;
            }
        }
    }
    ofFbo _fbo;
    ofParameter<float> _xPosition;
    ofParameter<float> _yPosition;
    ofParameter<float> _width;
    ofParameter<float> _height;

};
}; // namespace track
}; // namespace ofxLiveSet
