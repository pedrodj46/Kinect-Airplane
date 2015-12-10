#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        
        ofxKinect kinect;
        ofEasyCam easyCam;
    
        // punto in primo piano
        ofVec3f forepoint;
    
        // punto in primo piano colorato
        ofVec3f smoothedForepoint;
    
        // sfumature scie
        vector<ofVec3f> history;
    
        ofColor blue;
        ofColor green;
        ofColor red;
        ofColor white;
    
        vector<float> vectorx;
        vector<float> vectory;
        bool disegna;
        ofMesh l;

        float prova;
    
        float timeD, timeN;
    
};
