#include "ofApp.h"

#include "ofxUGen.h"

using namespace ofxUGen;

class MySynth : public ofxUGen::SynthDef
{
public:
	
	ofVec2f pos;
	
	UGen envgen;
	UGen amp;
    
	MySynth(int x,int y )
	{
		pos.x = x;
		pos.y = y;
		
		float freq = ofMap(pos.x, 0, ofGetHeight(), 0, 2000);
		float pan = ofMap(pos.y, 0, ofGetWidth(), -1, 1);
        
		Env env = Env::perc(0.5, 1.5, 0.3, EnvCurve::Sine);
		envgen = EnvGen::AR(env);
		envgen.addDoneActionReceiver(this);
		
		amp = SinOsc::AR(ofRandom(4.0), 0, 0.5, 0.5) * envgen;
		
		Out(
			Pan2::AR(SinOsc::AR(freq) * amp, pan)
            );
	}
	
	void draw()
	{
		ofFill();
		ofSetColor(100,100,255, amp.getValue() * 255);
		ofCircle(pos.x, pos.y, amp.getValue() * 100);
		
		ofNoFill();
		ofSetColor(255,100,100);
		ofCircle(pos.x, pos.y, amp.getValue() * 80);
	}
};

vector<MySynth*> synths;

const int WIDTH = 640;
const int HEIGHT = 480;


//--------------------------------------------------------------
void ofApp::setup(){
    
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(WIDTH,HEIGHT);
	
    colorImg.allocate(WIDTH,HEIGHT);
	grayImage.allocate(WIDTH,HEIGHT);
	grayBg.allocate(WIDTH,HEIGHT);
	grayDiff.allocate(WIDTH,HEIGHT);

	bLearnBakground = true;
	threshold = 255;
    
    s().setup();
}

//--------------------------------------------------------------
void ofApp::update(){
	ofBackground(100,100,100);

    bool bNewFrame = false;
    
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();

	if (bNewFrame){
        
        colorImg.setFromPixels(vidGrabber.getPixels(), WIDTH,HEIGHT);
        grayImage = colorImg;
		if (bLearnBakground == true){
			grayBg = grayImage;
			bLearnBakground = false;
		}

		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);

		contourFinder.findContours(grayDiff, 20, (WIDTH*HEIGHT)/3, 10, true);	// find holes
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
    grayImage.draw(0,0);
    contourFinder.draw(0,0);
    
    vector<MySynth*>::iterator it = synths.begin();
	while (it != synths.end())
	{
		MySynth *s = *it;
		s->draw();
		
		if (!s->isAlive())
		{
			it = synths.erase(it);
			delete s;
		}
		else
			it++;
	}
    if(contourFinder.nBlobs > 0){
        synths.push_back(new MySynth(contourFinder.blobs[0].centroid.x,contourFinder.blobs[0].centroid.y));
	    synths.back()->play();
    }
    
    for (int i = 0; i < synths.size(); i++)
		synths[i]->release();


	// finally, a report:
	ofSetHexColor(0xffffff);
	stringstream reportStr;
	reportStr << "bg subtraction and blob detection" << endl
			  << "press ' ' to capture bg" << endl
			  << "threshold " << threshold << " (press: +/-)" << endl
			  << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate();
	ofDrawBitmapString(reportStr.str(), 20, 400);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
