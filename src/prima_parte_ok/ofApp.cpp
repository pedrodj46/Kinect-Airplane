#include "ofApp.h"

const float FovX = 1.0144686707507438;
const float FovY = 0.78980943449644714;
const float XtoZ = tanf(FovX / 2) * 2;
const float YtoZ = tanf(FovY / 2) * 2;
const unsigned int Xres = 640;
const unsigned int Yres = 480;

ofVec3f ConvertProjectiveToRealWorld(float x, float y, float z) {
    return ofVec3f((x / Xres - .5f) * z * XtoZ,(y / Yres - .5f) * z * YtoZ,z);
    //return ofVec3f(x,y,z);
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    activeHome=true;
    activeAirplane=false;
    activeSound=false;
    
    airplane.loadModel("airplane.3DS");
    
    if(activeSound){
        sound.loadSound("audio.mp3");
        sound.setLoop(true);
        sound.setMultiPlay(false);
        sound.play();
    }
    
    sciaBianca.setMode(OF_PRIMITIVE_LINE_STRIP);
    sciaVerde.setMode(OF_PRIMITIVE_LINE_STRIP);
    sciaRossa.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    blue=ofColor(46, 60, 133);
    green=ofColor(54, 103, 53);
    red=ofColor(155, 35, 33);
    white=ofColor(233, 224, 210);
        
    ofSetVerticalSync(true);
    kinect.init();
    kinect.open();
    
    glEnable(GL_DEPTH_TEST);
    
    angle=0;
    angleY=0;
    kinect.setCameraTiltAngle(angle);
}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    
    // salvo la larghezza della finistra della kinect
    int width = kinect.getWidth();
    // salvo l'altezza della finestra della kinect
    int height = kinect.getHeight();
    
    // salvo la distanza in centimentri dalla kinect in un puntatore float
    float* distancePixels = kinect.getDistancePixels();
    
    // salvo il massimo numero in float
    forepoint.z = numeric_limits<float>::max();
    
    // creo due variabili forex e forey
    int forex, forey, i=0;
    // ciclo l'altezza
    for(int y = 0; y < height; y++){
        // ciclo la larghezza
        for(int x = 0; x < width; x++){
            
            // se l'elemento i di distancePixels  diverso da 0
            if(distancePixels[i]!=0){
                // se z  minore del valore di forepoint.z
                if(distancePixels[i] < forepoint.z){
                    // aggiorno le variabili forex e forey con i valori di x e y
                    forex = x;
                    forey = y;
                    // aggiorno il valore di forepoint con i punti x,y,z all'interno della funzione ConvertProkectToRealWolrd
                    forepoint = ConvertProjectiveToRealWorld(x,y,distancePixels[i]);
                }
            }
            // incremento i
            i++;
        }
    }
    
    // stampa a video la mesh cloud
    //cloud.drawFaces();
    
    int searchRadius = 128;
    ofVec3f sum;
    int count = 0;
    
    
    int maxSearchDistance = 1;
    
    for(int y = -searchRadius/2; y < +searchRadius/2; y++){
        for(int x = -searchRadius/2; x < +searchRadius/2;x++){
            int curx = x + forex;
            int cury = y + forey;
            
            
            if(curx > 0 && curx < width && cury > 0 && cury < height){
                int i = cury*width + curx;
                
                float curz = distancePixels[i];
                if(curz !=0 && abs(curz - forepoint.z) < maxSearchDistance){
                    
                    sum.z += curz;
                    sum.x += curx;
                    sum.y += cury;
                    count++;
                }
                
            }
        }
    }
    
    smoothedForepointOld=smoothedForepoint;
    
    if(count > 0){
        
        ofVec3f avg = sum / count;
        
        avg = ConvertProjectiveToRealWorld(avg.x, avg.y, avg.z);
        
        prova=avg.z;
        
        if(prova>=1400){
            avg = ConvertProjectiveToRealWorld(0,0,0);
            smoothedForepoint = avg;
        }
        else{
            if(smoothedForepoint==ofVec3f()){
                smoothedForepoint = avg;
            }
            else{
                smoothedForepoint.interpolate(avg,0.1);
            }
        }
    }
    else{
        ofVec3f avg = ConvertProjectiveToRealWorld(0,0,0);
        smoothedForepoint = avg;
    }
    
    
    if(sciaWhiteVector.size() > 50){
        sciaWhiteVector.erase(sciaWhiteVector.begin() + 0);
        sciaGreenVector.erase(sciaGreenVector.begin() + 0);
        sciaRedVector.erase(sciaRedVector.begin() + 0);
    }
    
    
    if(smoothedForepoint.z<1400){
        
        if(abs(smoothedForepoint.x-smoothedForepointOld.x)>2 || abs(smoothedForepoint.y-smoothedForepointOld.y)>2){
            deltaX=(smoothedForepoint.x-smoothedForepointOld.x)*-1;
            deltaY=smoothedForepoint.y-smoothedForepointOld.y;
            angleRadY=atan2(deltaY, deltaX);
            angleY=ofRadToDeg(angleRadY);
            
            if(angleY<0){
                angleY+=360;
            }
        }
        
        sciaBianca.clear();
        sciaVerde.clear();
        sciaRossa.clear();
        
        sciaWhiteVector.push_back(smoothedForepoint);
        ofVec3f smoothedForepointGreen(smoothedForepoint.x+ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.y+ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.z);
        sciaGreenVector.push_back(smoothedForepointGreen);
        ofVec3f smoothedForepointRed(smoothedForepoint.x-ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.y-ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.z);
        sciaRedVector.push_back(smoothedForepointRed);
        
        for(int i = 0; i < sciaWhiteVector.size(); i++){
            
            int alpha = ofMap(i, 0, 50, 0, 255);
            
            sciaBianca.addColor(ofColor(white, alpha));
            sciaVerde.addColor(ofColor(green, alpha));
            sciaRossa.addColor(ofColor(red, alpha));
            
            ofSetLineWidth(30);
            
            sciaBianca.addVertex(sciaWhiteVector[i]);
            sciaVerde.addVertex(sciaGreenVector[i]);
            sciaRossa.addVertex(sciaRedVector[i]);
        }
    }
    
    if(activeSound){
        sound.setVolume(abs(ofMap(smoothedForepoint.z, 700, 1400, 0, 1)-1));
        ofSoundUpdate();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // apro la camera
    easyCam.begin();
    
    ofEnableAlphaBlending();
    
    ofImage imgBG;
    imgBG.loadImage("sfondo.jpg");
    imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    
    ofPushStyle();
    ofImage imgLogo;
    imgLogo.loadImage("logo.jpg");
    imgLogo.draw(-ofGetWindowWidth()/2+10,ofGetWindowHeight()/2-70,1,45,60);
    
    ofSetColor(0,0,0);
    ofRect(-ofGetWindowWidth()/2, ofGetWindowHeight()/2-80,1,65,80);
    
    ofPopStyle();
    
    //kinect.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    
    ofPushStyle();
    ofPushMatrix();
    ofScale(-1, -1, -1);
    ofTranslate(0, 0, -1400);
    ofSetColor(blue);
    //ofCircle(smoothedForepoint.x, smoothedForepoint.y, smoothedForepoint.z, 10);
    
    airplane.setRotation(0, -90, 1, 0, 0);
    airplane.setRotation(1, angleY, 0, 1, 0);
    airplane.setScale(0.05, 0.05, 0.05);
    airplane.setPosition(smoothedForepoint.x, smoothedForepoint.y, smoothedForepoint.z-5);
    airplane.drawFaces();
    airplane.setPosition(smoothedForepoint.x+ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.y+ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.z-5);
    airplane.drawFaces();
    airplane.setPosition(smoothedForepoint.x-ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.y-ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), smoothedForepoint.z-5);
    airplane.drawFaces();
    
    if(sciaWhiteVector.size()>0){
        sciaBianca.drawVertices();
        sciaVerde.drawVertices();
        sciaRossa.drawVertices();
    }
    ofPopMatrix();
    ofPopStyle();
    
    ofPushStyle();
    ofSetColor(red);
    ofDrawBitmapString(" "+ofToString(smoothedForepoint.z), 0, 0);
    ofPopStyle();
    
    ofDisableAlphaBlending();
    
    easyCam.end();
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.setCameraTiltAngle(0);
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case OF_KEY_UP:
            angle++;
            if(angle>30){
                angle=30;
            }
            kinect.setCameraTiltAngle(angle);
            break;
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30){
                angle=-30;
            }
            kinect.setCameraTiltAngle(angle);
            break;
        case OF_KEY_DEL:
            angle=0;
            kinect.setCameraTiltAngle(angle);
            break;
        case 'o':
            kinect.setCameraTiltAngle(angle);
            kinect.open();
            break;
        case 'q':
            exit();
            break;
        case '1':
            kinect.setLed(ofxKinect::LED_GREEN);
            break;
        case '2':
            kinect.setLed(ofxKinect::LED_RED);
            break;
        case '3':
            kinect.setLed(ofxKinect::LED_BLINK_GREEN);
            break;
        case '4':
            kinect.setLed(ofxKinect::LED_OFF);
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
