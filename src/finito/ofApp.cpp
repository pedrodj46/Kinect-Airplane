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
    activeLoading=false;
    activeSound=false;
    activeDebug=false;
    activeBackgroundAirplane=false;
    activeBackgroundStory=false;
    activeStoryStop=false;
    
    activeStoryBack=0;
    activeLoadingCount=0;
    timeStoryAnimation=-35;
    
    photoNO=photoNE=photoSO=photoSE=photoSel=0;
    
    opensans.loadFont(ofToDataPath("OpenSans.ttf"), 26);
    
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
    
    int searchRadius = 128;
    ofVec3f sum;
    int count = 0;
    
    
    int maxSearchDistance = 100;
    
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
    
    
    if(activeHome){
        ofSetLineWidth(4);
        
        if(smoothedForepoint.x>100 && smoothedForepoint.x<420 && smoothedForepoint.y>-20 && smoothedForepoint.y<140){
            activeLoadingCount++;
            if(activeLoadingCount>60){
                activeHome=!activeHome;
                activeBackgroundHome=!activeBackgroundHome;
                activeLoading=!activeLoading;
                activeLoadingCount=1;
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
        
        if(smoothedForepoint.x>-420 && smoothedForepoint.x<-100 && smoothedForepoint.y>-20 && smoothedForepoint.y<140){
            activeLoadingCount++;
            if(activeLoadingCount>60){
                activeHome=!activeHome;
                activeBackgroundHome=!activeBackgroundHome;
                activeLoading=!activeLoading;
                activeLoadingCount=2;
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
    }
    
    if(activeAirplane){
    
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
        
        if(timeAirplaneMax<ofGetElapsedTimef()){
            activeBackgroundAirplane=!activeBackgroundAirplane;
            activeAirplane=!activeAirplane;
            activeLoading=!activeLoading;
            activeLoadingCount=0;
            timeLoading=ofGetElapsedTimef()+3;
        }
    }
    
    if(activeStory){
        ofSetLineWidth(4);
        
        if(smoothedForepoint.z>0 && smoothedForepoint.z<1400 && !activeStoryStop){
            if(smoothedForepoint.x>0 && smoothedForepoint.y>0){
                photoSO++;
                if(photoSO>100){
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    photoSel=1;
                    photoNO=photoNE=photoSO=photoSE=0;
                    activeStoryStop=!activeStoryStop;
                    activeStoryBack=0;
                }
            }
            else if(smoothedForepoint.x>0 && smoothedForepoint.y<0){
                photoNO++;
                if(photoNO>100){
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    photoSel=2;
                    photoNO=photoNE=photoSO=photoSE=0;
                    activeStoryStop=!activeStoryStop;
                    activeStoryBack=0;
                }
            }
            else if(smoothedForepoint.x<0 && smoothedForepoint.y>0){
                photoSE++;
                if(photoSE>100){
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    photoSel=3;
                    photoNO=photoNE=photoSO=photoSE=0;
                    activeStoryStop=!activeStoryStop;
                    activeStoryBack=0;
                }
            }
            else if(smoothedForepoint.x<0 && smoothedForepoint.y<0){
                photoNE++;
                if(photoNE>100){
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    photoSel=4;
                    photoNO=photoNE=photoSO=photoSE=0;
                    activeStoryStop=!activeStoryStop;
                    activeStoryBack=0;
                }
            }
        }
        else if(smoothedForepoint.z<=0 && !activeStoryStop){
            cout << activeStoryBack << "\n";
            activeStoryBack++;
            if(activeStoryBack>50){
                activeBackgroundStory=!activeBackgroundStory;
                activeStory=!activeStory;
                activeLoading=!activeLoading;
                activeLoadingCount=0;
                activeStoryBack=0;
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
    }
    
    if(activeLoading){
        if(timeLoading<ofGetElapsedTimef()){
            activeLoading=!activeLoading;
            
            if(activeLoadingCount==0){
                if(activeBackgroundAirplane){
                    activeBackgroundAirplane=!activeBackgroundAirplane;
                }
                if(activeBackgroundStory){
                    activeBackgroundStory=!activeBackgroundStory;
                }
                activeHome=!activeHome;
                activeLoadingCount=0;
            }
            
            if(activeLoadingCount==1){
                activeBackgroundHome=!activeBackgroundHome;
                activeAirplane=!activeAirplane;
                timeAirplane=ofGetElapsedTimef();
                timeAirplaneMax=ofGetElapsedTimef()+30;
                activeLoadingCount=0;
            }
            
            if(activeLoadingCount==2){
                activeBackgroundHome=!activeBackgroundHome;
                activeStory=!activeStory;
                activeLoadingCount=0;
            }
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
    
    ofPushStyle();
    ofImage imgLogo;
    imgLogo.loadImage("logo.jpg");
    imgLogo.draw(-32,ofGetWindowHeight()/2-90,1,64,80);
    ofSetColor(blue);
    ofRect(-ofGetWindowWidth()/2, ofGetWindowHeight()/2-100,1,ofGetWindowWidth(),100);
    ofPopStyle();
    
    if(activeAirplane){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,2);
        ofSetColor(255,255,255);
        opensans.setLineHeight(40);
        int timer=static_cast<int>(ofGetElapsedTimef()-timeAirplane);
        string str;
        if(timer<10){
            str="00:0"+ofToString(timer)+"/00:30";
        }
        else{
            str="00:"+ofToString(timer)+"/00:30";
        }
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,ofGetWindowWidth()/2-rectStr.width-100,ofGetWindowHeight()/2-rectStr.height-30);
        ofPopStyle();
        ofPopMatrix();
    }
    
    if(activeDebug){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,2);
        ofSetColor(255,255,255);
        string str;
        str=ofToString(smoothedForepoint);
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-ofGetWindowWidth()/2+50,ofGetWindowHeight()/2-rectStr.height-30);
        ofPopStyle();
        ofPopMatrix();
    }
    
    ofImage imgBG;
    if(activeHome || activeBackgroundHome){
        ofPushStyle();
        ofSetColor(255,255,255,100);
        imgBG.loadImage("bg-home.jpg");
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
        ofPopStyle();
    }
    else if(activeAirplane || activeBackgroundAirplane){
        imgBG.loadImage("bg-airplane.jpg");
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    }
    else if(activeStory || activeBackgroundStory){
        imgBG.loadImage("bg-story.jpg");
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    }
    
    //kinect.draw((ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    
    ofPushStyle();
    ofPushMatrix();
    ofScale(-1, -1, -1);
    ofTranslate(0, 0, -1400);
    ofSetColor(blue);
    
    if(activeHome || activeStory){
        ofCircle(smoothedForepoint.x, smoothedForepoint.y, smoothedForepoint.z, 10);
    }
    
    if(activeAirplane){
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
    }
    
    ofPopMatrix();
    ofPopStyle();
    
    if(activeHome){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,1);
        ofSetColor(red);
        opensans.setLineHeight(40);
        string str="Posiziona il cursore a sinistra o a destra!";
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-rectStr.width/2,ofGetWindowHeight()/2-160);
        ofPopStyle();
        ofPopMatrix();
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(-ofGetWindowWidth()/3,-30,1);
        ofSetColor(red);
        str="  Fai sfrecciare\n  le tue\n  frecce tricolori!";
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-rectStr.width/2,0);
        //ofSetColor(blue);
        ofNoFill();
        //ofCircle(0,-100,2,30);
        ofSetColor(white);
        ofRectRounded(-rectStr.width/2-20,-rectStr.height*2,400,rectStr.height*4-rectStr.height/2,5);
        ofPopStyle();
        ofPopMatrix();
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(ofGetWindowWidth()/4,-30,1);
        ofSetColor(red);
        str="  Scopri\n  la storia\n  delle frecce!";
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-rectStr.width/2,0);
        //ofSetColor(blue);
        ofNoFill();
        //ofCircle(0,-100,2,30);
        ofSetColor(white);
        ofRectRounded(-rectStr.width/2-20,-rectStr.height*2,400,rectStr.height*4-rectStr.height/2,5);
        ofPopStyle();
        ofPopMatrix();
    }
    
    if(activeStory){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,2);
        ofSetColor(255,255,255);
        opensans.setLineHeight(30);
        string str="Posiziona il cursore\nsopra una immagine!";
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-ofGetWindowWidth()/2+50,ofGetWindowHeight()/2-rectStr.height/2-13);
        
        str="Per tornare indietro, esci\ndal raggio visivo della kinect!";
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,ofGetWindowWidth()/2-rectStr.width-50,ofGetWindowHeight()/2-rectStr.height/2-13);
        ofPopStyle();
        ofPopMatrix();
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,2);
        alphaStory=0;
        if(timeStoryAnimation+35>=ofGetElapsedTimef()){
            if(activeStoryStop && timeStoryAnimation>=ofGetElapsedTimef()){
                alphaStory = ofMap(ofGetElapsedTimef(), timeStoryAnimation-3, timeStoryAnimation, 0, 255);
            }
            else{
                alphaStory=255;
            }
        }
        else{
            activeStoryStop=false;
            timeStoryAnimation=-35;
        }
        ofSetColor(0,0,0,alphaStory);
        string str1;
        switch(photoSel){
            case 1:
                str1="Le Frecce Tricolori hanno tre programmi di esecuzione delle acrobazie:\nalto, basso, e piatto, a seconda delle condizioni meteorologiche e\nalle caratteristiche dell'area dell'esibizione. Da alcuni anni,\nquando possibile, le esibizioni della PAN si chiudono con la\nformazione al completo che disegna nel cielo un tricolore lungo cinque\nkm mentre dagli altoparlanti a terra la voce di Luciano Pavarotti\nintona il finale di Nessun dorma per l'intera durata del passaggio.";
                break;
            case 2:
                str1="Le Frecce Tricolori, il cui nome per esteso e' Pattuglia Acrobatica\nNazionale, costituente il 313esimo Gruppo Addestramento Acrobatico,\nsono la pattuglia acrobatica nazionale (PAN)\ndell'Aeronautica Militare Italiana, nate nel 1961\nin seguito alla decisione dell'Aeronautica Militare\ndi creare un gruppo permanente per l'addestramento\nall'acrobazia aerea collettiva dei suoi piloti.";
                break;
            case 3:
                str1="Sono numerosi i riconoscimenti attribuiti alle Frecce Tricolori.\nNel 2005 vengono premiate per la migliore esibizione al Royal\nInternation Air Tattoo in Giordania e al Salone Internazionale\nAeronautico di Mosca, ricevono la medaglia d'argento al merito\naeronautico. Nel 2008, durante la celebrazione della 'Giornata della\nQualita' Italiana', il Presidente della Repubblica ha consegnato il\n'Premio Leonardo' alle Frecce Tricolori e nel 2010 hanno ricevuto dal\nMinistro degli Esteri il 'Winning Italy Award' in riconoscimento\ndell'Alta Eccellenza raggiunta per promuovere e valorizzare\nl'immagine dell'Italia nel Mondo.";
                break;
            case 4:
                str1="Con dieci aerei, di cui nove in formazione e uno solista,\nsono la pattuglia acrobatica piu' numerosa del mondo,\ned il loro programma di volo, comprendente una ventina\ndi acrobazie e della durata di circa mezz'ora, le ha rese\nle piu' famose. Dal 1982 utilizzano come velivolo gli Aermacchi\nMB.339 A/PAN MLU, e la sede e' l'aeroporto di Rivolto (UD).";
                break;
        }
        opensans.setLineHeight(50);
        rectStr=opensans.getStringBoundingBox(str1, 0, 0);
        ofRectRounded(-ofGetWindowWidth()/2+20,-ofGetWindowHeight()/2+20,ofGetWindowWidth()-40,ofGetWindowHeight()-140,5);
        ofTranslate(0,0,3);
        ofSetColor(255,255,255,alphaStory);
        opensans.drawString(str1,-ofGetWindowWidth()/2+40,ofGetWindowHeight()/2-170);
        ofPopStyle();
        ofPopMatrix();
    }
    
    if(activeLoading){
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(0,0,1);
        alphaLoading=0;
        if(timeLoading>=ofGetElapsedTimef()){
            alphaLoading = ofMap(ofGetElapsedTimef(), timeLoading-3, timeLoading, 0, 255);
        }
        else{
            alphaLoading=255;
        }
        ofSetColor(0,0,0,alphaLoading);
        ofRect(-ofGetWindowWidth()/2,-ofGetWindowHeight()/2,ofGetWindowWidth(), ofGetWindowHeight()-100);
        ofTranslate(0,0,2);
        ofSetColor(255,255,255,alphaLoading);
        string str="caricamento...";
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        opensans.drawString(str,-rectStr.width/2,0);
        ofPopStyle();
        ofPopMatrix();
    }
    
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
        case OF_KEY_BACKSPACE:
            angle=0;
            kinect.setCameraTiltAngle(angle);
            break;
        case 'd':
            activeDebug=!activeDebug;
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
        case '0':
            activeHome=!activeHome;
            activeBackgroundHome=!activeBackgroundHome;
            activeLoading=!activeLoading;
            activeLoadingCount=2;
            timeLoading=ofGetElapsedTimef()+3;
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
