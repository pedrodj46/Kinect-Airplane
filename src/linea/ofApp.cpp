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
    
    disegna=false;
    l.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    blue=ofColor(46, 60, 133);
    green=ofColor(54, 103, 53);
    red=ofColor(155, 35, 33);
    white=ofColor(233, 224, 210);
    
    timeN=ofGetElapsedTimef()+3;
        
    ofSetVerticalSync(true);
    kinect.init();
    kinect.open();
}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    
    // salvo la larghezza della finistra della kinect
    int width = kinect.getWidth();
    // salvo l'altezza della finestra della kinect
    int height = kinect.getHeight();
    
    // salvo la distanza in centimentri dalla kinect in una matrice float
    float* distancePixels = kinect.getDistancePixels();
    
    // attivare il point cloud
    ofMesh cloud;
    cloud.setMode(OF_PRIMITIVE_POINTS);
    
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
                // aggiungo un vertice alla mesh cloud con i punti x,y,z all'interno della funzione ConvertProjectToRealWorld
                cloud.addVertex(ConvertProjectiveToRealWorld(x, y, distancePixels[i]));
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
    
    
    if(count > 0){
        
        ofVec3f avg = sum / count;
        
        avg = ConvertProjectiveToRealWorld(avg.x, avg.y, avg.z);
        
        prova=avg.z;
        
        if(avg.z>=1600){
            avg=ConvertProjectiveToRealWorld(0,0,500);
        }
        
        if(smoothedForepoint==ofVec3f()){
            smoothedForepoint = avg;
        }
        else{
            smoothedForepoint.interpolate(avg,0.1);
        }
        
    }
    else{
        ofVec3f avg=ConvertProjectiveToRealWorld(0,0,0);
        smoothedForepoint = avg;
    }
    
    vectorx.push_back(smoothedForepoint.x);
    vectory.push_back(smoothedForepoint.y);
    int somma=0;
    
    if(!disegna && prova<1600){
        /*if(vectorx.size()==50 && vectory.size()==50){
        
            for(int i=0;i<vectorx.size();i++){
                if(vectorx[0]-vectorx[i]>5)
                    somma++;
            }
            for(int i=0;i<vectory.size();i++){
                if(vectory[0]-vectory[i]>5)
                    somma++;
            }
        
            if(somma==0){
                //cout << "entra \n";
                green.set(255,0,0);
                disegna=true;
            }
            else{
                green.set(0,255,0);
            }
        
            vectorx.clear();
            vectory.clear();
        }*/
        
        if(smoothedForepoint.z<750 && timeN<ofGetElapsedTimef()){
            timeD=ofGetElapsedTimef()+3;
            green.set(255,0,0);
            disegna=true;
            
        }
        
        if(history.size()>0){
            for(int h=0;h<10;h++){
                int alpha=ofMap(h, 0, 100, 0, 255);
                l.setColor(h, ofColor(blue, alpha));
            }
            history.erase(history.begin());
            l.removeVertex(0);
        }
        
    }
    else{
        l.clear();
        
        history.push_back(smoothedForepoint);
        
        /*if(vectorx.size()==80 && vectory.size()==80){
            
            for(int i=0;i<vectorx.size();i++){
                if(vectorx[0]-vectorx[i]>5)
                    somma++;
            }
            for(int i=0;i<vectory.size();i++){
                if(vectory[0]-vectory[i]>5)
                    somma++;
            }
            if(somma==0){
                green.set(0,255,0);
                disegna=false;
            }
            
            vectorx.clear();
            vectory.clear();
        }*/
        
        if(smoothedForepoint.z<750 && timeD<ofGetElapsedTimef()){
            timeN=ofGetElapsedTimef()+3;
            green.set(0,255,0);
            disegna=false;
            
        }
        
        for(int h=0;h<history.size();h++){
            l.addColor(blue);
            ofSetLineWidth(2);
            l.addVertex(history[h]);
        }

    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // apro la camera
    easyCam.begin();
    
    ofEnableAlphaBlending();
    
    //kinect.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    
    ofPushStyle();
    ofPushMatrix();
    ofScale(-1, -1, -1);
    ofTranslate(0, 0, -1000);
    ofSetColor(green);
    ofCircle(smoothedForepoint.x, smoothedForepoint.y, smoothedForepoint.z, 10);
    if(disegna){
        l.draw();
    }
    else{
        if(history.size()>0){
            l.draw();
        }
    }
    ofPopMatrix();
    ofPopStyle();
    
    ofSetColor(red);
    ofDrawBitmapString("z: "+ofToString(prova), 0, 0);
    
    ofPushStyle();
    ofNoFill();
    ofSetColor(blue);
    ofRect((ofGetWindowWidth()/2)-30, (ofGetWindowHeight()/2)-30, 30, 30);
    ofPopStyle();
    
    ofDisableAlphaBlending();
    
    
    easyCam.end();
}

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key==' '){
        ofImage image;
        image.grabScreen(0, 0, 500, 500);
        image.saveImage("prova.jpg");
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
