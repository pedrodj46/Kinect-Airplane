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
    
        // kinect e easyCam
        ofxKinect kinect;
        ofEasyCam easyCam;
    
        // larghezza e altezza della finestra della kinect
        int kinectWidth;
        int kinectHeight;
    
        // punto in primo piano
        ofVec3f forepoint;
        // punto in primo piano colorato
        ofVec3f visibleForepoint;
        // punto che salva i punti precedenti
        ofVec3f visibleForepointOld;
    
        // sfumature scie
        vector<ofVec3f> wakeWhiteVector, wakeGreenVector, wakeRedVector;
    
        // sei colori utilizzati nel progetto: nero assoluto, biancoa assoluto, blu, verde, rosso e bianco
        ofColor blue;
        ofColor green;
        ofColor red;
        ofColor white;
        ofColor blackAbsolute;
        ofColor whiteAbsolute;
    
        // tre mesh che servono per creare le scie degli aerei
        ofMesh wakeWhite, wakeGreen, wakeRed;

        // controllo della profonditˆ
        float control;
        // angolazione della kinect
        int angle;
    
        // massima distanza dalla kinect
        int distance;
    
        // utilizzate per il calcolo dell'angolo partendo dalle coordinate
        float deltaX;
        float deltaY;
        float angleRadY;
        float angleY;
    
        // variabile per il caricamento del modello 3D
        ofxAssimpModelLoader airplane;
    
        // variabile per il caricamento del font esterno
        ofTrueTypeFont opensans;
    
        // variabile per attivare il suono
        ofSoundPlayer sound;
    
        // booleane per il cambiamento delle varie videate
        bool activeHome;
        bool activeAirplane;
        bool activeStory;
        bool activeLoading;
    
        // booleane per il debug di variabili
        bool activeDebug;
        bool activeDebugDistance;
    
        // booleane per inserire il background nella schermata di caricamento
        bool activeBackgroundHome;
        bool activeBackgroundAirplane;
        bool activeBackgroundStory;
    
        // booleana per stoppare il contatore activeStoryStop e quindi ritardare il ritorno alla videata Home
        bool activeStoryStop;
    
        // booleana per attivare il suono
        bool activeSound;
    
        // contatore per passare da una videata all'altra
        int activeLoadingCount;
        // contatore per tornare alla videata Home da quella Story
        int activeStoryBack;
    
        // variabili per il riconoscimento della foto selezionata
        int photoNO;
        int photoNE;
        int photoSO;
        int photoSE;
    
        // foto selezionata nella videata Story
        int photoSel;
    
        // variabili per il calcolo dell'alpha. Con queste variabili si crea l'effetto Fade In
        int alphaLoading;
        int alphaStory;
    
        // float per il calcolo del tempo
        float timeLoading;
        float timeAirplane;
        float timeAirplaneMax;
        float timeStoryAnimation;
    
};
