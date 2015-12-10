#include "ofApp.h"

// constati float e int
const float FovX = 1.0144686707507438;
const float FovY = 0.78980943449644714;
const float XtoZ = tanf(FovX / 2) * 2;
const float YtoZ = tanf(FovY / 2) * 2;
const unsigned int Xres = 640;
const unsigned int Yres = 480;

// funzione ConvertProjectiveToRealWorld che prende in input le tre coordinate x,y e z per traslarle dalla visuale proiettiva a quella reale. Trasla il punto in input da laterale a centrale
ofVec3f ConvertProjectiveToRealWorld(float x, float y, float z) {
    return ofVec3f((x / Xres - .5f) * z * XtoZ,(y / Yres - .5f) * z * YtoZ,z);
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    // imposto tutte le variabili booleane a false tranne activeHome che è la prima schermata che si vedrà quando parte il progetto
    activeHome=true;
    activeAirplane=false;
    activeLoading=false;
    activeDebug=false;
    activeDebugDistance=false;
    activeBackgroundAirplane=false;
    activeBackgroundStory=false;
    activeStoryStop=false;
    activeSound=false;
    
    // setto le variabili di conteggio a 0
    activeStoryBack=0;
    activeLoadingCount=0;
    // setto la variabile di conteggio dell'animazione e del tempo totale a -35
    timeStoryAnimation=-35;
    
    // setto tutti i contatoti delle foto e la varibile photoSel per la foto selezionata a 0
    photoNO=photoNE=photoSO=photoSE=photoSel=0;
    
    //carico la font Open Sans di Google e imposto la grandezza a 26
    opensans.loadFont(ofToDataPath("OpenSans.ttf"), 26);
    
    // carico il modello 3D "airplane.3DS"
    airplane.loadModel("airplane.3DS");
    
    // setto le mesh con il parametro "OF_PRIMITIVE_LINE_STRIP" per collegare i vertici a coppie
    wakeWhite.setMode(OF_PRIMITIVE_LINE_STRIP);
    wakeGreen.setMode(OF_PRIMITIVE_LINE_STRIP);
    wakeRed.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // setto i sei colori
    blue=ofColor(46, 60, 133);
    green=ofColor(54, 103, 53);
    red=ofColor(155, 35, 33);
    white=ofColor(233, 224, 210);
    blackAbsolute=ofColor(0,0,0);
    whiteAbsolute=ofColor(255,255,255);
    
    // collego e apro il collegamento con la kinect
    kinect.init();
    kinect.open();
    
    // distanza massima dal punto più vicino alla kinect
    distance=1400;
    
    // abilito OpenGL per la profondità
    glEnable(GL_DEPTH_TEST);
    
    // setto a 0 le due variabili angolo
    angle=0;
    angleY=0;
    // setto l'angolazione della kinect a 0
    kinect.setCameraTiltAngle(angle);
    
    // salvo la larghezza della finistra della kinect
    kinectWidth=kinect.getWidth();
    // salvo l'altezza della finestra della kinect
    kinectHeight=kinect.getHeight();
    
    // carico il suono degli aerei
    sound.loadSound("airplane-sound.wav");
    // setto il suono in loop
    sound.setLoop(true);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    // aggiorno la kinect
    kinect.update();
    
    /* Trovo il punto più vicino alla kinect */
    
    // salvo la distanza in centimentri dalla kinect in un puntatore float
    float* distancePixels=kinect.getDistancePixels();
    
    // inizializzo la variabile forepoint.z a 0
    forepoint.z=0;
    
    // dichiaro due variabili forex e forey, che serviranno per salvare le coordinate del punto più vicino alla kinect
    int forex, forey;
    // dichiaro e inizializzo la variabile i per scansionare tutto il vettore distancePixels
    int i=0;
    // con un doppio ciclo for, ciclo l'altezza e la larghezza della finestra della kinect per trovare le coordinate del punto più vicino alla kinect
    for(int y=0;y<kinectHeight;y++){
        for(int x=0;x<kinectWidth;x++){
            // se l'elemento i di distancePixels è diverso da 0
            if(distancePixels[i]!=0){
                // se forepoint.z è uguale a 0, quindi è il primo giro che entra
                if(forepoint.z==0){
                    // salvo il valore di x e y nelle rispettive variabili forex e forey
                    forex=x;
                    forey=y;
                    // salvo il valore di forepoint con i punti x,y,z all'interno della funzione ConvertProkectToRealWolrd
                    forepoint=ConvertProjectiveToRealWorld(x,y,distancePixels[i]);
                }
                // se forepoint.z è diverso da 0, quindi è già stata salvata una coordinata, e se distancePixels di i è minore del valore di forepoint.z salvato, quindi il punto è più vicino di quello salvato
                else if(distancePixels[i]<forepoint.z){
                    // aggiorno le variabili forex e forey con i valori di x e y
                    forex=x;
                    forey=y;
                    // aggiorno il valore di forepoint con i punti x,y,z all'interno della funzione ConvertProkectToRealWolrd
                    forepoint=ConvertProjectiveToRealWorld(x,y,distancePixels[i]);
                }
            }
            // incremento i
            i++;
        }
    }
    
    /* Cerco intorno al punto più vicino alla kinect altri punti simili */
    
    // dichiaro e inizializzo la variabile searchRadius, con la quale vado alla ricerca di punti vicini al punto forepoint.z
    int searchRadius=100;
    // dichiato la variabile tridimensionale sum, con la quale si sommeranno tutti i punti vicini a forepoint.z
    ofVec3f sum;
    // dichiaro e inizializzo a 0 la variabie count
    int count=0;
    
    // dichiaro e inizializzo la variabile maxSearchDistance per limitare in profondità la ricerca di punti vicini
    int maxSearchDistance=100;
    
    // con due cicli for, scansiono il quadrato con centro forex e forey per trovare punti vicini a forepoint
    for(int y=forey-searchRadius/2;y<forey+searchRadius/2;y++){
        for(int x=forex-searchRadius/2;x<forex+searchRadius/2;x++){
            // se è all'interno della videata della kinect
            if(x>0 && x<kinectWidth && y>0 && y<kinectHeight){
                // trovo l'indice del vettore partendo dalle coordinate x e y. Moltiplico la variabile y per la larghezza della finestra della kinect e sommo la variabile x
                int i=y*kinectWidth+x;
                // salvo il valore di distancePixels all'indice i
                float z=distancePixels[i];
                
                // se z è diverso da 0 e il valore assoluto della differenza tra z e forepoint.z è inferiore al valore di maxSearchDistance
                if(z!=0 && abs(z-forepoint.z)<maxSearchDistance){
                    // sommo all'interno della variabile tridimensionale sum i valori di x,y e z e incremento count
                    sum.z+=z;
                    sum.x+=x;
                    sum.y+=y;
                    count++;
                }
                
            }
        }
    }
    
    // salvo visibleForepoint prima di essere aggiornato nella variabile visibleForepointOld
    visibleForepointOld=visibleForepoint;
    
    /* Faccio la media dei punti simili e trovo il punto centrale più vicino alla kinect */
    
    // se la variabile count è maggiore di 0, quindi sono presenti punti all'interno dell'area del forepoint
    if(count>0){
        // divido la variabile sum con il valore di count e creo la media dei punti. Salvo il dato nella variabile average
        ofVec3f average=sum/count;
        // converto il valore di average con la funzione ConvertProjectiveToRealWorld
        average=ConvertProjectiveToRealWorld(average.x, average.y, average.z);
        
        // salvo la z di average nella variabile control
        control=average.z;
        
        // se control è maggiore di distance
        if(control>=distance){
            // imposto la variabile average a 0
            average=ConvertProjectiveToRealWorld(0,0,0);
            // salvo il valore di average nella variabile visibleForepoint
            visibleForepoint = average;
        }
        // se control è inferiore a distance
        else{
            // se visibleForepoint è di tipo ofVec3f
            if(visibleForepoint==ofVec3f()){
                // salvo il valore di average nella variabile visibleForepoint
                visibleForepoint=average;
            }
            // se visibleForepoint non è di tipo ofVec3f
            else{
                // eseguo l'interpolazione tra visibleForepoint e average di 0.1 per avere un effetto lineare e non scattoso della rilevazione del punto più vicino alla kinect
                visibleForepoint.interpolate(average,0.1);
            }
        }
    }
    // se count è uguale a 0
    else{
        // imposto la variabile average a 0
        ofVec3f average=ConvertProjectiveToRealWorld(0,0,0);
        //salvo il valore di average nella variabile visibleForepoint
        visibleForepoint=average;
    }
    
    /* Se siamo nella videata Home, calcolo dov'è il puntatore per selezionare le due voci */
    
    // se activeHome è true, ossia quando siamo nella videata Home
    if(activeHome){
        // setta lo spessore della linea a 4
        ofSetLineWidth(4);
        
        // se visibleForepoint di x è maggiore di 100 e minore di 420 e visibleForepoint è maggiore di -20 e inferiore a 140, ossia quando il cursore è nel rettangolo di sinistra
        if(visibleForepoint.x>100 && visibleForepoint.x<420 && visibleForepoint.y>-20 && visibleForepoint.y<140){
            // incrementa la variabile activeLoadingCount
            activeLoadingCount++;
            // se la variabile activeLoadingCount è maggiore di 60
            if(activeLoadingCount>60){
                // setta activeHome a false
                activeHome=!activeHome;
                // setta activeBackgroundHome a true, ossia mostra il background della videata Home mentre entra in Fade In la videata di Loading
                activeBackgroundHome=!activeBackgroundHome;
                // setta activeLoading a true, ossia cambia la videata Home con quella di Loading
                activeLoading=!activeLoading;
                // imposta activeLoadingCount a 1, che serve per riconoscere quale videata far vedere
                activeLoadingCount=1;
                // setta timeLoading il tempo di apertura del progetto maggiorato di 3 secondi
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
        
        // se visibleForepoint di x è maggiore -420 e minore di -100 e visibleForepoint è maggiore di -20 e inferiore a 140, ossia quando il cursore è nel rettangolo di destra
        if(visibleForepoint.x>-420 && visibleForepoint.x<-100 && visibleForepoint.y>-20 && visibleForepoint.y<140){
            // incrementa la variabile activeLoadingCount
            activeLoadingCount++;
            // se la variabile activeLoadingCount è maggiore di 60
            if(activeLoadingCount>60){
                // setta activeHome a false
                activeHome=!activeHome;
                // setta activeBackgroundHome a true, ossia mostra il background della videata Home mentre entra in Fade In la videata di Loading
                activeBackgroundHome=!activeBackgroundHome;
                // setta activeLoading a true, ossia cambia la videata Home con quella di Loading
                activeLoading=!activeLoading;
                // imposta activeLoadingCount a 2, che serve per riconoscere quale videata far vedere
                activeLoadingCount=2;
                // setta timeLoading il tempo di apertura del progetto maggiorato di 3 secondi
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
    }
    
    /* Se siamo nella videata Airplane, calcolo l'angolazione degli aerei e la loro scia */
    
    // se activeAirplane è true
    if(activeAirplane){
        // se la dimensione del vettore wakeWhiteVector è maggiore di 50
        if(wakeWhiteVector.size() > 50){
            // tolgo il primo valore del vettore wakeWhiteVector, wakeGreenVector e wakeRedVector. Questo per avere la scia degli aeroplani sempre di dimensione 50
            wakeWhiteVector.erase(wakeWhiteVector.begin() + 0);
            wakeGreenVector.erase(wakeGreenVector.begin() + 0);
            wakeRedVector.erase(wakeRedVector.begin() + 0);
        }
        
        // se il valore di z di visibleForepoint è minore del valore di distance
        if(visibleForepoint.z<distance){
            // se il valore assoluto della differenza tra il valore attuale di visibleForepoint e quello di visibleForepointOld sia dell'asse x che dell'asse y è maggiore di 2, quindi se c'è stato un cambiamento della posizione degli aeroplani
            if(abs(visibleForepoint.x-visibleForepointOld.x)>2 || abs(visibleForepoint.y-visibleForepointOld.y)>2){
                // calcolo il deltaX facendo la differenza tra visibleForepoint di x e visibleForepointOld di x moltiplicati per -1
                deltaX=(visibleForepoint.x-visibleForepointOld.x)*-1;
                // calcolo il deltaY facendo la differenza tra visibleForepoint di x e visibleForepointOld
                deltaY=visibleForepoint.y-visibleForepointOld.y;
                // calcolo l'arcotangente tra deltaY e deltaX e trovo l'angolo in radianti
                angleRadY=atan2(deltaY, deltaX);
                // converto i radianti in gradi
                angleY=ofRadToDeg(angleRadY);
                
                // se i gradi sono negativi, vengono riportati positivi
                if(angleY<0){
                    // sommo alla variabile angleY 360
                    angleY+=360;
                }
            }
            
            // cancello completamente le mesh wakeWhite, wakeGreen e wakeRed
            wakeWhite.clear();
            wakeGreen.clear();
            wakeRed.clear();
            
            // aggiungo alla fine del vettore wakeWhiteVector la variabile visibleForepoint
            wakeWhiteVector.push_back(visibleForepoint);
            // calcolo il punto da salvare nel vettore wakeGreenVector partendo dalla coordinata visibleForepoint traslata di 30
            ofVec3f visibleForepointRed(visibleForepoint.x+ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.y+ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.z);
            // aggiungo alla fine del vettore wakeRedVector la variabile visibleForepointRed
            wakeRedVector.push_back(visibleForepointRed);
            // calcolo il punto da salvare nel vettore wakeRedVector partendo dalla coordinata visibleForepoint traslata di -30
            ofVec3f visibleForepointGreen(visibleForepoint.x-ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.y-ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.z);
            // aggiungo alla fine del vettore wakeGreenVector la variabile visibleForepointGreen
            wakeGreenVector.push_back(visibleForepointGreen);
            
            // ciclo l'intero vettore wakeWhiteVector
            for(int i=0;i<wakeWhiteVector.size();i++){
                
                // dichiaro e inizializzo la variabile alpha mappando il valore i in entrata tra 0 e 50 e in uscita tra 0 e 255
                int alpha=ofMap(i,0,wakeWhiteVector.size(),0,255);
                
                // aggiungo il colore alla mesh con il relativo alpha
                wakeWhite.addColor(ofColor(white, alpha));
                wakeGreen.addColor(ofColor(green, alpha));
                wakeRed.addColor(ofColor(red, alpha));
                
                // setto la larghezza della linea a 30
                ofSetLineWidth(30);
                
                // aggiungo il vertice alle mesh con il proprio vettore con indice i
                wakeWhite.addVertex(wakeWhiteVector[i]);
                wakeGreen.addVertex(wakeGreenVector[i]);
                wakeRed.addVertex(wakeRedVector[i]);
            }
        }
        
        // se timeAirplaneMax è maggiore del tempo di avvio del progetto, quindi è finito il tempo della videata Airplane
        if(timeAirplaneMax<ofGetElapsedTimef()){
            // cambio stato a activeBackgroundAirplane
            activeBackgroundAirplane=!activeBackgroundAirplane;
            // cambio stato a activeAirplane
            activeAirplane=!activeAirplane;
            // cambio stato a activeLoading
            activeLoading=!activeLoading;
            // setto activeLoadingCount a 0
            activeLoadingCount=0;
            // in timeLoading  salvo il tempo di avvio del progetto maggiorato di 3 secondi
            timeLoading=ofGetElapsedTimef()+3;
            // controllo se activeSound è true
            if(activeSound){
                // cambio stato ad activeSound
                activeSound=!activeSound;
                // fermo la riproduzione di sound
                sound.stop();
            }
        }
    }
    
    /* Se siamo nella videata Story, calcolo quale immagine si sta selezionando e incremento la sua variabile per mostrare il testo in draw */
    
    // se activeStory è true
    if(activeStory){
        // setto la larghezza della linea a 4
        ofSetLineWidth(4);
        
        // se visibleForepoint.z è maggiore di 0 e minore di distance e activeStoryStop è false, si può scegliere la foto
        if(visibleForepoint.z>0 && visibleForepoint.z<distance && !activeStoryStop){
            // se visibleForepoint.x e visibleForepoint.y è maggiore di 25, si sta selezionando la foto in basso a sinistra
            if(visibleForepoint.x>0 && visibleForepoint.y>25){
                // incremento la variabile photoSO
                photoSO++;
                // se photoSO è maggiore di 100, è stata selezionata la foto in basso a sinistra
                if(photoSO>100){
                    // salvo in timeStoryAnimation il tempo di avvio del progetto maggiorato di 3 secondi
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    // photoSel uguale ad 1 per capire quale testo mostrare
                    photoSel=1;
                    // imposto tutte le variabili contatori delle foto a 0
                    photoNO=photoNE=photoSO=photoSE=0;
                    // cambio stato ad activeStoryStop per fermare l'incremento delle variabili contatori delle foto
                    activeStoryStop=!activeStoryStop;
                    // setto activeStoryBack a 0 per posticipare il ritorno alla videata Home
                    activeStoryBack=0;
                }
            }
            // se invece visibleForepoint.x è maggiore di 0 e visibleForepoint.y è minore di 25, si sta selezionando la foto in alto a sinistra
            else if(visibleForepoint.x>0 && visibleForepoint.y<25){
                // incremento photoNO
                photoNO++;
                // se photoNO è maggiore di 100, è stata selezionata la foto in alto a sinistra
                if(photoNO>100){
                    // salvo in timeStoryAnimation il tempo di avvio del progetto maggiorato di 3 secondi
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    // photoSel uguale ad 2 per capire quale testo mostrare
                    photoSel=2;
                    // imposto tutte le variabili contatori delle foto a 0
                    photoNO=photoNE=photoSO=photoSE=0;
                    // cambio stato ad activeStoryStop per fermare l'incremento delle variabili contatori delle foto
                    activeStoryStop=!activeStoryStop;
                    // setto activeStoryBack a 0 per posticipare il ritorno alla videata Home
                    activeStoryBack=0;
                }
            }
            // se invece visibleForepoint.x è minore di 0 e visibleForepoint.y è maggiore di 25, si sta selezionando la foto in basso a destra
            else if(visibleForepoint.x<0 && visibleForepoint.y>25){
                // incremento photoSE
                photoSE++;
                // se photoSE è maggiore di 100, è stata selezionata la foto in basso a destra
                if(photoSE>100){
                    // salvo in timeStoryAnimation il tempo di avvio del progetto maggiorato di 3 secondi
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    // photoSel uguale ad 3 per capire quale testo mostrare
                    photoSel=3;
                    // imposto tutte le variabili contatori delle foto a 0
                    photoNO=photoNE=photoSO=photoSE=0;
                    // cambio stato ad activeStoryStop per fermare l'incremento delle variabili contatori delle foto
                    activeStoryStop=!activeStoryStop;
                    // setto activeStoryBack a 0 per posticipare il ritorno alla videata Home
                    activeStoryBack=0;
                }
            }
            // se invece visibleForepoint.x è minore di 0 e visibleForepoint.y è minore di 25, si sta selezionando la foto in alto a destra
            else if(visibleForepoint.x<0 && visibleForepoint.y<25){
                // incremento photoNE
                photoNE++;
                // se photoNE è maggiore di 100, è stata selezionata la foto in alto a destra
                if(photoNE>100){
                    // salvo in timeStoryAnimation il tempo di avvio del progetto maggiorato di 3 secondi
                    timeStoryAnimation=ofGetElapsedTimef()+3;
                    // photoSel uguale ad 4 per capire quale testo mostrare
                    photoSel=4;
                    // imposto tutte le variabili contatori delle foto a 0
                    photoNO=photoNE=photoSO=photoSE=0;
                    // cambio stato ad activeStoryStop per fermare l'incremento delle variabili contatori delle foto
                    activeStoryStop=!activeStoryStop;
                    // setto activeStoryBack a 0 per posticipare il ritorno alla videata Home
                    activeStoryBack=0;
                }
            }
        }
        // se invece visibleForepoint.z è minore o uguale a 0 e activeStoryStop è false
        else if(visibleForepoint.z<=0 && !activeStoryStop){
            // incremento activeStoryBack
            activeStoryBack++;
            // se activeStoryBack è maggiore di 100, si torna alla schermata Home
            if(activeStoryBack>100){
                // cambio stato a activeBackgroundStory
                activeBackgroundStory=!activeBackgroundStory;
                // cambio stato a activeStory
                activeStory=!activeStory;
                // cambio stato a activeLoading
                activeLoading=!activeLoading;
                // setto activeLoadingCount a 0
                activeLoadingCount=0;
                // setto activeStoryBack a 0
                activeStoryBack=0;
                // salvo in timeLoading il tempo di avvio del progetto maggiorato di 3 secondi
                timeLoading=ofGetElapsedTimef()+3;
            }
        }
    }
    
    
    /* Quando si cambia videata, la videata di Loading compare in Fade In da alpha 0 ad alpha 255 in nero con la scritta 'caricamento' */
    
    // se activeLoading è true
    if(activeLoading){
        // se timeLoading è minore del tempo di apertura del progetto, quindi il tempo di attesa è finito
        if(timeLoading<ofGetElapsedTimef()){
            // cambio stato ad activeLoading
            activeLoading=!activeLoading;
            
            // se activeLoadingCount è uguale a 0, viene visualizzata la videata Home
            if(activeLoadingCount==0){
                // se activeBackgroundAirplane è true
                if(activeBackgroundAirplane){
                    // setto activeBackgroundAirplane false
                    activeBackgroundAirplane=!activeBackgroundAirplane;
                }
                // se activeBackgroundStory è true
                if(activeBackgroundStory){
                    // setto activeBackgroundStory false
                    activeBackgroundStory=!activeBackgroundStory;
                }
                // cambio stato a activeHome
                activeHome=!activeHome;
                // setto activeLoadingCount a 0
                activeLoadingCount=0;
            }
            
            // se activeLoadingCount è uguale a 1, viene visualizzata la videata Airplane
            if(activeLoadingCount==1){
                // cambio stato a activeBackgroundHome
                activeBackgroundHome=!activeBackgroundHome;
                // cambio stato a activeAirplane
                activeAirplane=!activeAirplane;
                // setto timeAirplane con il tempo di apertura del progetto
                timeAirplane=ofGetElapsedTimef();
                // setto timeAirplaneMax con il tempo di apertura del progetto maggiorato di 30
                timeAirplaneMax=ofGetElapsedTimef()+30;
                // setto activeLoadingCount a 0
                activeLoadingCount=0;
            }
            
            // se activeLoadingCount è uguale a 2, viene visualizzata la videata Story
            if(activeLoadingCount==2){
                // cambio stato a activeBackgroundHome
                activeBackgroundHome=!activeBackgroundHome;
                // cambio stato a activeStory
                activeStory=!activeStory;
                // setto activeLoadingCount a 0
                activeLoadingCount=0;
            }
        }
    }
    
    /* Se viene attivato il suono nella videata Airplane, calcolo il volume che deve avere l'audio in base alla distanza del punto più vicino alla kinect */
    
    // se activeSound è true
    if(activeSound){
        // setto il volume dell'audio con il valore assoluto del valore di visibleForepoint.z mappandolo tra 700 e distance e in uscita tra 0 e 1, cambiandolo di segno, per far cambiare il volume in base alla distanza dell'aereo
        sound.setVolume(abs(ofMap(visibleForepoint.z, 700, distance, 0, 1)-1));
        // aggiorno il suono
        ofSoundUpdate();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    // creo la camera easyCam per avere interazione 3D
    easyCam.begin();
    
    // abilito la sfumatura del colora tramite il parametro alpha
    ofEnableAlphaBlending();
    
    /* Disegno il logo e la barra rettangolare blu in alto */
    
    // salvo il vecchio stile
    ofPushStyle();
    // dichiaro la varabile imgLogo che conterrà il logo delle Frecce Tricolori
    ofImage imgLogo;
    // carico l'immgine logo.jpg
    imgLogo.loadImage("logo.jpg");
    // disegno il logo in alto centrato
    imgLogo.draw(-32,ofGetWindowHeight()/2-90,1,64,80);
    // setto il colore blu
    ofSetColor(blue);
    // creo un rettangolo in alto che contiene il logo lungo tutta la finestra
    ofRect(-ofGetWindowWidth()/2, ofGetWindowHeight()/2-100,1,ofGetWindowWidth(),100);
    // ripristino il vecchio stile
    ofPopStyle();
    
    /* Se siamo nella videata Airplane, mostro il timer in alto a destra vicino al logo */
    
    // se activeAirplane è true
    if(activeAirplane){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo tutto di z=2
        ofTranslate(0,0,2);
        // setto il colore bianco
        ofSetColor(whiteAbsolute);
        // setto l'interlinea a 40
        opensans.setLineHeight(40);
        // dichiaro e inizializzo la variabile timer con il valore intero della differenza tra il tempo di avvio del progetto e il valore della variabile timeAirplane
        int timer=static_cast<int>(ofGetElapsedTimef()-timeAirplane);
        // dichiaro la stringa str che conterrà il timer
        string str;
        // se il valore di timer è inferiore a 10
        if(timer<10){
            // mostro il timer con lo 0 davanti
            str="00:0"+ofToString(timer)+"/00:30";
        }
        // se timer è maggiore o uguale a 10
        else{
            // mostro il timer senza 0 davanti
            str="00:"+ofToString(timer)+"/00:30";
        }
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in alto a destra vicino al logo
        opensans.drawString(str,ofGetWindowWidth()/2-rectStr.width-100,ofGetWindowHeight()/2-rectStr.height-30);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    /* Se è attivo il debug, mostro in alto a sinistra le coordinate di visibleForepoint */
    
    // se activeDebug è true
    if(activeDebug){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo tutto di z=2
        ofTranslate(0,0,2);
        // setto il colore bianco assoluto
        ofSetColor(whiteAbsolute);
        // dichiato la stringa str
        string str;
        // converto in stringa la variabile visibleForepoint
        str=ofToString(visibleForepoint);
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in alto a sinistra vicino al logo
        opensans.drawString(str,-ofGetWindowWidth()/2+50,ofGetWindowHeight()/2-rectStr.height-30);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    /* Se è attivo il debug della distanza, mostro in alto a sinistra la distanza massima tra la kinect e il punto più vicino ad assa */
    
    // se activeDebugDistance è true
    if(activeDebugDistance){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo tutto di z=2
        ofTranslate(0,0,2);
        // setto il colore bianco assoluto
        ofSetColor(whiteAbsolute);
        // dichiato la stringa str
        string str;
        // converto in stringa la variabile distance
        str=ofToString(distance);
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in alto a sinistra vicino al logo
        opensans.drawString(str,-ofGetWindowWidth()/2+50,ofGetWindowHeight()/2-rectStr.height-30);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    /* In base alla videata in cui mi trovo, disegnerò il suo background */
    
    // dichiato la variabile imgBG
    ofImage imgBG;
    // se activeHome o activeBackgroundHome sono true
    if(activeHome || activeBackgroundHome){
        // salvo il vecchio stile
        ofPushStyle();
        // setto il colore bianco assoluto con alpha uguale a 100
        ofSetColor(whiteAbsolute,100);
        // carico l'immagine bg-home.jpg
        imgBG.loadImage("bg-home.jpg");
        // disegno l'immagine grande come tutto la schermata
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
        // ripristino il vecchio stile
        ofPopStyle();
    }
    // invece se activeAirplane o activeBackgroundAirplane sono true
    else if(activeAirplane || activeBackgroundAirplane){
        // carico l'immagine bg-airplane.jpg
        imgBG.loadImage("bg-airplane.jpg");
        // disegno l'immagine grande come tutto la schermata
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    }
    // invece se activeStory o activeBackgroundStory sono true
    else if(activeStory || activeBackgroundStory){
        // carico l'immagine bg-story.jpg
        imgBG.loadImage("bg-story.jpg");
        // disegno l'immagine grande come tutto la schermata
        imgBG.draw(-(ofGetWindowWidth()/2),-(ofGetWindowHeight()/2),ofGetWindowWidth(),ofGetWindowHeight());
    }
    
    // salvo il vecchio stile
    ofPushStyle();
    // salvo le coordinate vecchie
    ofPushMatrix();
    // scalo tutti gli assi di -1
    ofScale(-1,-1,-1);
    // traslo nell'asse z del valore negati della variabile distance
    ofTranslate(0, 0, -distance);
    // setto il colore blu
    ofSetColor(blue);
    
    /* Se siamo nelle videate Home o Story, mostriamo il puntatore nello schermo */
    
    // se activeHome o activeStory è true
    if(activeHome || activeStory){
        // disegna un cerchio di raggio 10 con posizione visibleForepoint
        ofCircle(visibleForepoint.x, visibleForepoint.y, visibleForepoint.z, 10);
    }
    
    /* Se siamo nella videata Home, disegno gli aerei e la loro scia */
    
    // se activeAirplane è true
    if(activeAirplane){
        // setto la rotazione del modello 3D a -90° nell'asse x
        airplane.setRotation(0, -90, 1, 0, 0);
        // setto una seconda rotazione del modello 3D dinamica del valore di angleY
        airplane.setRotation(1, angleY, 0, 1, 0);
        // scalo il modello 3D di 0.05 in tutti gli assi
        airplane.setScale(0.05, 0.05, 0.05);
        // setto al posizione dell'aeroplano in mezzo con la variabile visibleForepoint
        airplane.setPosition(visibleForepoint.x, visibleForepoint.y, visibleForepoint.z-5);
        // disegno il modello 3D
        airplane.drawFaces();
        // setto al posizione dell'aeroplano a destra con la variabile visibleForepoint traslato di 30
        airplane.setPosition(visibleForepoint.x+ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.y+ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.z-5);
        // disegno il modello 3D
        airplane.drawFaces();
        // setto al posizione dell'aeroplano a sinistra con la variabile visibleForepoint traslato di 30
        airplane.setPosition(visibleForepoint.x-ofMap(sin(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.y-ofMap(cos(ofDegToRad(angleY)),-1,1,-30,30), visibleForepoint.z-5);
        // disegno il modello 3D
        airplane.drawFaces();
        
        // se il vettore wakeWhiteVector è maggiore di 0
        if(wakeWhiteVector.size()>0){
            // disegno le mesh wakeWhite, wakeGreen e wakeRed
            wakeWhite.drawVertices();
            wakeGreen.drawVertices();
            wakeRed.drawVertices();
        }
    }
    // ripristino le vecchie coordinate
    ofPopMatrix();
    // ripristino il vecchio stile
    ofPopStyle();
    
    /* Se siamo nella videata Home, stampo le stringhe e i box per selezionare le altre videate */
    
    if(activeHome){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo di z=1
        ofTranslate(0,0,1);
        // setto il colore rosso
        ofSetColor(red);
        // setto l'interlinea a 40
        opensans.setLineHeight(40);
        // inizializzo la stringa str
        string str="Posiziona il cursore a sinistra o a destra!";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in alto sotto il logo
        opensans.drawString(str,-rectStr.width/2,ofGetWindowHeight()/2-160);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
        
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo per creare il rettangolo a sinistra
        ofTranslate(-ofGetWindowWidth()/3,-30,1);
        // setto il colore rosso
        ofSetColor(red);
        // cambio valore alla stringa str
        str="  Fai sfrecciare\n  le tue\n  frecce tricolori!";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa
        opensans.drawString(str,-rectStr.width/2,0);
        // tolgo il riempimento
        ofNoFill();
        // setto il colore bianco
        ofSetColor(white);
        // disegno un rettangolo con gli angoli arrotondati intorno alla stringa str
        ofRectRounded(-rectStr.width/2-20,-rectStr.height*2,400,rectStr.height*4-rectStr.height/2,5);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
        
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo per creare il rettangolo a destra
        ofTranslate(ofGetWindowWidth()/4,-30,1);
        // setto il colore rosso
        ofSetColor(red);
        // cambio valore alla stringa str
        str="  Scopri\n  la storia\n  delle frecce!";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa
        opensans.drawString(str,-rectStr.width/2,0);
        // tolgo il riempimento
        ofNoFill();
        // setto il colore bianco
        ofSetColor(white);
        // disegno un rettangolo con gli angoli arrotondati intorno alla stringa str
        ofRectRounded(-rectStr.width/2-20,-rectStr.height*2,400,rectStr.height*4-rectStr.height/2,5);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    /* Se siamo nella videata Story, stampo le stringhe e la descrizione della foto selezionata */
    
    if(activeStory){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo di z=2
        ofTranslate(0,0,2);
        // setto il colore bianco assoluto
        ofSetColor(whiteAbsolute);
        // setto l'interlina a 30
        opensans.setLineHeight(30);
        // inizializzo la stringa str
        string str="Posiziona il cursore\nsopra una immagine!";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in altro a sinistra vicino al logo
        opensans.drawString(str,-ofGetWindowWidth()/2+50,ofGetWindowHeight()/2-rectStr.height/2-13);
        
        // cambio valore alla stringa str
        str="Per tornare indietro, esci\ndal raggio visivo della kinect!";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa in alto a destra vicino al logo
        opensans.drawString(str,ofGetWindowWidth()/2-rectStr.width-50,ofGetWindowHeight()/2-rectStr.height/2-13);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
        
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo di z=2
        ofTranslate(0,0,2);
        // inizializzo il valore di alphaStory a 0
        alphaStory=0;
        // se timeStoryAnimation maggiorato di 35 secondi è maggiore o uguale al tempo di avvio del progetto, quindi è visibile il testo della foto
        if(timeStoryAnimation+35>=ofGetElapsedTimef()){
            // se activeStoryStop è true e timeStoryAnimation è maggiore o uguale al tempo di avvio del progetto
            if(activeStoryStop && timeStoryAnimation>=ofGetElapsedTimef()){
                // mappo il tempo di avvio del progetto con timeStoryAnimation meno 3 secondi e timeStoryAnimation in entrata e in uscita da 0 a 255. Questo serve per creare il valore dell'alpha
                alphaStory=ofMap(ofGetElapsedTimef(), timeStoryAnimation-3, timeStoryAnimation, 0, 255);
            }
            // se activeStoryStop è false o timeStoryAnimation è minore al tempo di avvio del progetto
            else{
                // setta alphaStory a 255
                alphaStory=255;
            }
        }
        // se timeStoryAnimation maggiorato di 35 secondi è minore al tempo di avvio del progetto, quindi il tempo di visualizzazione del testo della foto è finito
        else{
            // setto activeStoryStop a false
            activeStoryStop=false;
            // setto timeStoryAnimation a -35
            timeStoryAnimation=-35;
        }
        // setto il colore a nero assoluto con l'alpha a alphaStory
        ofSetColor(blackAbsolute,alphaStory);
        // dichiaro la stringa str1
        string str1;
        // se il valore di photoSel è uguale ad un caso, eseguilo
        switch(photoSel){
            // se photoSel è uguale a 1
            case 1:
                // inizializzo la stringa str1
                str1="Le Frecce Tricolori hanno tre programmi di esecuzione delle acrobazie:\nalto, basso, e piatto, a seconda delle condizioni meteorologiche e\nalle caratteristiche dell'area dell'esibizione. Da alcuni anni,\nquando possibile, le esibizioni della PAN si chiudono con la\nformazione al completo che disegna nel cielo un tricolore lungo cinque\nkm mentre dagli altoparlanti a terra la voce di Luciano Pavarotti\nintona il finale di Nessun dorma per l'intera durata del passaggio.";
                break;
            // se photoSel è uguale a 2
            case 2:
                // inizializzo la stringa str1
                str1="Le Frecce Tricolori, il cui nome per esteso e' Pattuglia Acrobatica\nNazionale, costituente il 313esimo Gruppo Addestramento Acrobatico,\nsono la pattuglia acrobatica nazionale (PAN)\ndell'Aeronautica Militare Italiana, nate nel 1961\nin seguito alla decisione dell'Aeronautica Militare\ndi creare un gruppo permanente per l'addestramento\nall'acrobazia aerea collettiva dei suoi piloti.";
                break;
            // se photoSel è uguale a 3
            case 3:
                // inizializzo la stringa str1
                str1="Sono numerosi i riconoscimenti attribuiti alle Frecce Tricolori.\nNel 2005 vengono premiate per la migliore esibizione al Royal\nInternation Air Tattoo in Giordania e, al Salone Internazionale\nAeronautico di Mosca, ricevono la medaglia d'argento al merito\naeronautico. Nel 2008, durante la celebrazione della 'Giornata della\nQualita' Italiana', il Presidente della Repubblica ha consegnato il\n'Premio Leonardo' alle Frecce Tricolori e nel 2010 hanno ricevuto dal\nMinistro degli Esteri il 'Winning Italy Award' in riconoscimento\ndell'Alta Eccellenza raggiunta per promuovere e valorizzare\nl'immagine dell'Italia nel Mondo.";
                break;
            // se photoSel è uguale a 4
            case 4:
                // inizializzo la stringa str1
                str1="Con dieci aerei, di cui nove in formazione e uno solista,\nsono la pattuglia acrobatica piu' numerosa del mondo,\ned il loro programma di volo, comprendente una ventina\ndi acrobazie e della durata di circa mezz'ora, le ha rese\nle piu' famose. Dal 1982 utilizzano come velivolo gli Aermacchi\nMB.339 A/PAN MLU, e la sede e' l'aeroporto di Rivolto (UD).";
                break;
        }
        // setto l'interlinea a 50
        opensans.setLineHeight(50);
        // creo un rettangolo della grandezza della variabile str1 per ricavare la larghezza e lunghezza della stringa
        rectStr=opensans.getStringBoundingBox(str1, 0, 0);
        // disegno un rettangolo con i bordi arrotondati grande quasi tutta la finestra meno il rettangolo del logo e un pò di margine sopra e sotto
        ofRectRounded(-ofGetWindowWidth()/2+20,-ofGetWindowHeight()/2+20,ofGetWindowWidth()-40,ofGetWindowHeight()-140,5);
        // traslo di z=3
        ofTranslate(0,0,3);
        // setto il colore bianco assoluto con l'alpha a alphaStory
        ofSetColor(whiteAbsolute,alphaStory);
        // disegno la stringa all'interno del rettangolo precedentemente creato
        opensans.drawString(str1,-ofGetWindowWidth()/2+40,ofGetWindowHeight()/2-170);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    /* Se siamo nella schermata di Loading, calcolo l'alpha e stampo la scritta 'caricamento' */
    
    if(activeLoading){
        // salvo le coordinate vecchie
        ofPushMatrix();
        // salvo il vecchio stile
        ofPushStyle();
        // traslo di z=1
        ofTranslate(0,0,1);
        // setto la variabile alphaLoading a 0
        alphaLoading=0;
        // se timeLoading è maggiore o uguale al tempo di avvio del progetto
        if(timeLoading>=ofGetElapsedTimef()){
            // mappo il risultato del tempo di avvio del progetto tra il valore di timeLoading decrementato di 3 secondi e il valore di timeLoading in entrata e in uscita tra 0 e 255. Serve per creare il valore dell'alpha.
            alphaLoading=ofMap(ofGetElapsedTimef(), timeLoading-3, timeLoading, 0, 255);
        }
        // se timeLoading è minore del tempo di avvio del progetto
        else{
            // setto il valore di alphaLoading a 255
            alphaLoading=255;
        }
        // setto il colore nero assoluto con alpha alphaLoading
        ofSetColor(blackAbsolute,alphaLoading);
        // creo un rettangolo grando come tutta la grandezza della finestra meno il rettangolo del logo
        ofRect(-ofGetWindowWidth()/2,-ofGetWindowHeight()/2,ofGetWindowWidth(), ofGetWindowHeight()-100);
        // traslo di z=2
        ofTranslate(0,0,2);
        // setto il colore bianco assoluto con alpha alphaLoading
        ofSetColor(whiteAbsolute,alphaLoading);
        // inizializzo la stringa str
        string str="caricamento...";
        // creo un rettangolo della grandezza della variabile str per ricavare la larghezza e lunghezza della stringa
        ofRectangle rectStr=opensans.getStringBoundingBox(str, 0, 0);
        // disegno la stringa al centro della finestra
        opensans.drawString(str,-rectStr.width/2,0);
        // ripristino il vecchio stile
        ofPopStyle();
        // ripristino le vecchie coordinate
        ofPopMatrix();
    }
    
    // disabilito la sfumatura
    ofDisableAlphaBlending();
    // chiudo la EasyCam
    easyCam.end();
}

//--------------------------------------------------------------
void ofApp::exit(){
    // setta l'angolazione della kinect a 0
    kinect.setCameraTiltAngle(0);
    // chiudi il collegamento con la kinect
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // se il tasto premuto (key) è uguale ad uno di questi casi, esegui le azioni
    switch(key){
        // se key è uguale al tasto su
        case OF_KEY_UP:
            // incrementa angle
            angle++;
            // se angle è maggiore di 30, massima angolazione della kinect
            if(angle>30){
                // setta angle a 30
                angle=30;
            }
            // setta l'angolazione della kinect con il valore di angle
            kinect.setCameraTiltAngle(angle);
            break;
        // se key è uguale al tasto giù
        case OF_KEY_DOWN:
            // decrementa angle
            angle--;
            // se angle è minore di -30, minima angolazione della kinect
            if(angle<-30){
                // setta angle a -30
                angle=-30;
            }
            // setta l'angolazione della kinect con il valore di angle
            kinect.setCameraTiltAngle(angle);
            break;
        // se key è uguale al tasto backspace
        case OF_KEY_BACKSPACE:
            // setta angle a 0
            angle=0;
            // setta l'angolazione della kinect con il valore di angle
            kinect.setCameraTiltAngle(angle);
            break;
        // se key è uguale al tasto d
        case 'd':
            // cambia stato a activeDebug
            activeDebug=!activeDebug;
        // se key è uguale al tasto o
        case 'o':
            // setta l'angolazione della kinect con il valore di angle
            kinect.setCameraTiltAngle(angle);
            // apri il collegamento con la kinect
            kinect.open();
            break;
        // se key è uguale al tasto q
        case 'q':
            // richiama il metodo exit()
            exit();
            break;
        // se key è uguale al tasto 1
        case '1':
            // imposta il led verde alla kinect
            kinect.setLed(ofxKinect::LED_GREEN);
            break;
        // se key è uguale al tasto 2
        case '2':
            // imposta il led rosso alla kinect
            kinect.setLed(ofxKinect::LED_RED);
            break;
        // se key è uguale al tasto 3
        case '3':
            // imposta il led verde lampeggiante alla kinect
            kinect.setLed(ofxKinect::LED_BLINK_GREEN);
            break;
        // se key è uguale al tasto 4
        case '4':
            // spegni il led della kinect
            kinect.setLed(ofxKinect::LED_OFF);
            break;
        // se key è uguale al tasto +
        case '+':
            // incrementa di 25 la variabile distance
            distance+=25;
            break;
        // se key è uguale al tasto -
        case '-':
            // decrementa di 25 la variabile distance
            distance-=25;
            break;
        // se key è uguale al tasto m
        case 'm':
            // cambia stato a activeDebugDistance
            activeDebugDistance=!activeDebugDistance;
            break;
        // se key è uguale al tasto s
        case 's':
            // se activeAirplane è true
            if(activeAirplane){
                // se activeSound è true
                if(activeSound){
                    // cambio stato ad activeSound
                    activeSound=!activeSound;
                    // fermo la riproduzione di sound
                    sound.stop();
                }
                // se activeSound è false
                else{
                    // cambio stato a activeSound
                    activeSound=!activeSound;
                    // inizio la riproduzione di sound
                    sound.play();
                }
            }
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
