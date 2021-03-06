#pragma once
#include "clips/graphic.h"
namespace clips {
    class bezierVertex : public ofxLiveSet::clip::graphic {
    public:
        typedef std::shared_ptr<bezierVertex> pointer;
        static pointer create(std::string name="")
        {
            return std::make_shared<bezierVertex>(name);
        }
        bezierVertex(std::string name = "bezierVertex") : graphic(name){
            _name = name;
            _active.setName(_name);
            
            _primaryColor.set("primaryColor", ofColor::white);
            _amount.set("amount", 5, 1, 50);

            _parameters.add(_primaryColor);
            _parameters.add(_shaderEnabled);
            _shaderEnabled = true;

            _shader.load("bezierVertex/shaders/noise.vert", "bezierVertex/shaders/noise.frag");
        }
        void update(){
            ofEnableAlphaBlending();
            begin();
            float x0 = -_width/4;
            float y0 = 0;
            float x1 = x0+_width/2+50*cos(ofGetElapsedTimef()*1.0f);
            float y1 = y0+100*sin(ofGetElapsedTimef()/3.5f);
            float x2 = 100+30*cos(ofGetElapsedTimef()*2.0f);
            float y2 = y0+100*sin(ofGetElapsedTimef());
            float x3 = x0+_width/2 - 50;
            float y3 = y0;
        
            
            ofFill();
            ofSetColor(_primaryColor);
            ofBeginShape();
            ofVertex(x0,y0);
            ofBezierVertex(x1,y1,x2,y2,x3,y3);
            ofEndShape();
            
            
            ofEnableAlphaBlending();
                ofFill();
                ofSetColor(0,0,0,40);
                ofDrawCircle(x0,y0,4);
                ofDrawCircle(x1,y1,4);
                ofDrawCircle(x2,y2,4);
                ofDrawCircle(x3,y3,4);
            ofDisableAlphaBlending();

            end();
            setNewFrame();
        }

        ofParameter<ofColor> _primaryColor;
        ofParameter<int> _amount;
    };
};