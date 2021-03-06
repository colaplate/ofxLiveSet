#include "ofApp.h"

#include "./clips/dmx/within.h"
#include "./clips/dmx/anchor.h"
#include "./clips/dmx/schwanensee.h"
#include "./clips/dmx/firn.h"
#include "./clips/dmx/wind.h"
#include "./clips/dmx/newClip.h"
#include "./clips/dmx/strobe.h"
#include "./clips/dmx/utils/still.h"
#include "./clips/dmx/utils/midi2dmx.h"

ofApp::ofApp() : _project(ofxLiveSet::project::create())
{
    setApplicationName("klangLichtStrom");
    setApplicationVersion("1.2.0");

    _session = _project->_sessions.back();
}

void ofApp::setup()
{
    ofSetBackgroundColor(16, 16, 16);

    _dmx.connect("/dev/cu.usbserial-EN160415");
    _dmx.setChannels(18);
    _visualisation.setup(&_dmx);

    auto lightBulbsTrack = ofxLiveSet::track::dmx::create("light bulbs");
    auto strobeTrack = ofxLiveSet::track::dmx::create("strobe");
    auto hazerTrack = ofxLiveSet::track::dmx::create("hazer");
    auto utilsTrack = ofxLiveSet::track::dmx::create("utils");
    auto afterShowTrack = ofxLiveSet::track::dmx::create("after show");

    lightBulbsTrack->addClip(clips::within::create())->setup();
    lightBulbsTrack->addClip(clips::anchor::create())->setup();
    lightBulbsTrack->addClip(clips::schwanensee::create())->setup();
    lightBulbsTrack->addClip(clips::firn::create())->setup();
    lightBulbsTrack->addClip(clips::wind::create())->setup();
    lightBulbsTrack->addClip(clips::newClip::create())->setup();

    strobeTrack->addClip(clips::strobe::create(21, 22), 3)->setup();

    utilsTrack->addClip(clips::still::create(), 7)->setup();
    utilsTrack->addClip(clips::midi2dmx::create())->setup();

    utilsTrack->mute();
    afterShowTrack->mute();

    _session->addTrack(lightBulbsTrack);
    _session->addTrack(strobeTrack);
    _session->addTrack(hazerTrack);
    _session->addTrack(utilsTrack);
    _session->addTrack(afterShowTrack);

    for (auto track : _session->_tracks)
    {
        if (auto dmxTrack = std::dynamic_pointer_cast<ofxLiveSet::track::dmx>(track))
        {
            dmxTrack->setup(&_dmx);
        }
    }

    _session->setup();
    _session->setupGui();
    _session->openOscControlInPort(9000);
    _session->openMidiMapperInPort(1);
    _session->openMidiInPort(0);
    _session->stop();
    _session->_mqttSynchroniserEnabled = false;

    _session->renameScene(0, "within");
    _session->renameScene(1, "anchor");
    _session->renameScene(2, "schwanensee");
    _session->renameScene(3, "firn");
    _session->renameScene(4, "wind");
    _session->renameScene(5, "new");
    for (auto i = 6; i <= 8; i++)
    {
        _session->renameScene(i, "");
    }
}

void ofApp::exit()
{
    for (auto i = 1; i <= 22; i++)
    {
        _dmx.setLevel(i, 0);
    }
    _dmx.update();
}

void ofApp::update()
{
    _visualisation.update();
#ifdef SENDDMX
    _dmx.update();
#endif
}

void ofApp::draw()
{
    _visualisation.draw(
        ofGetWidth() / 3 * 2,
        ofGetHeight() / 2,
        ofGetWidth() / 3, ofGetHeight() / 2);
    _session->drawGui();
}

void ofApp::keyPressed(int key)
{
    _session->onKeyPressed(key);
    if (key == 'f')
    {
        ofToggleFullscreen();
    }
}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}