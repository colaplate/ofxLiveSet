#pragma once
#include "./soundReactiveDmx.h"

namespace clips {
    class firn : public soundReactiveDmx {
    public:
        firn() : soundReactiveDmx(), _lastIndex(0){
            _name = "firn";
            _channel.set("channel", 1, 1, 512);
            _amount.addListener(this, &firn::onAmountChange);
            _amount.set("amount", 16, 1, 16);
            
            _minValue.set("minValue", 0, 0, 255);
            _maxValue.set("maxValue", 100, 0, 255);
            _speed.set("speed", 1, 0, 1);
            _threshold.set("threshold", .5, 0, 1);
            _peakEnergyDebounceTime.set("debounce", 50, 0, 100);
            _minDistance.set("minDistance", 7, 1, 12);
            
            _active.setName(_name);
            
            _parameters.add(_channel);
            _parameters.add(_amount);
            _parameters.add(_minValue);
            _parameters.add(_maxValue);
            _parameters.add(_speed);
            _parameters.add(_threshold);
            _parameters.add(_peakEnergyDebounceTime);
            _parameters.add(_minDistance);
            
            _timestamp = ofGetElapsedTimeMillis();
        }
        
        void update(){
            auto timestamp = ofGetElapsedTimeMillis();
            
            for(auto i = 0; i < _amount; i++) {
                if(timestamp - _timestamps[i] < 1000){
                    _values[i] = ofMap(timestamp, _timestamps[i], _timestamps[i] + 1000, _maxValue, _minValue);
                    std::pair<int, int> value(_channel+i, _values[i]);
                    _valueChangeEvent.notify(value);
                }
            }
        }
        
        void setPeakEnergy(int analyserId, float value) {
            if(value < _threshold){return;}
            auto timestamp = ofGetElapsedTimeMillis();
            if(timestamp - _timestamp < _peakEnergyDebounceTime){
                ofLogNotice()<<"firn return";
                return;
            }
            _timestamp = timestamp;
            
            auto i = ofRandom(_amount);
            while(abs(_lastIndex - i) < _minDistance){
                i = ofRandom(_amount);
            }
            _lastIndex = i;
            _timestamps[i] = ofGetElapsedTimeMillis();
            _values[i] = _maxValue*value;
            
        }
        
        void onAmountChange(int & value){
            _values.resize(value);
            _timestamps.resize(value);
        }
        
        ofParameter<int> _channel;
        ofParameter<int> _amount;
        ofParameter<int> _value;
        std::vector<int> _values;
        ofParameter<int> _minValue;
        ofParameter<int> _maxValue;
        
        ofParameter<float> _speed;
        ofParameter<float> _threshold;
        ofParameter<int> _peakEnergyDebounceTime;
        ofParameter<int> _minDistance;
        
        u_int64_t _timestamp;
        std::vector<u_int64_t> _timestamps;
        int _lastIndex;
    };
};
