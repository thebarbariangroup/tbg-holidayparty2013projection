#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "cinder/Timeline.h"
#include "cinder/Rand.h"
#include "cinder/Camera.h"
#include "cinder/gl/Light.h"
#include "cinder/CinderMath.h"
#include "GridPhoto.h"
#include "PhotoStream.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ProjectionApp : public AppNative {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void resize( );
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
	void loadNewPhoto();
	void setPhotos();
	void swapGridPhoto();
	PhotoRef getRandomPhoto();
	
	shared_ptr<PhotoStream>			mPhotoStream;
//	gl::Texture						mBackground;
	gl::Texture						mSquareShadow;
	gl::Texture						testPhoto;
	vector<GridPhoto*>				gridPhotos;
	vector<PhotoRef>				mLoadedPhotos;
	CueRef							mLoadPhotoTimer;	// Timer for loading a new photo
	CueRef							mSwapImageTimer;	// Timer for swapping a photo in one of the grids
	bool							mStarted;			// whether the grid has been populated or not
	CameraPersp						mCam;
	Vec3f							mEye, mCenter, mUp;
};

void ProjectionApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 60 );
}

void ProjectionApp::setup()
{
	mStarted = false;
	
//	mBackground = gl::Texture( loadImage( loadResource( "background.png" ) ) );
	mSquareShadow = gl::Texture( loadImage( loadResource( "square_shadow.png" ) ) );
	
	int cols = 5;
	int rows = 3;
	int amt = cols * rows;
	Vec2f size = Vec2f( 240.0f, 240.0f );
	
	// the list if urls that that the app will be pulling photos from. It goes through them one by one as it needs to photos to fill the queue
//	vector<string> urls = {"http://localhost:5000/getPhoto", "http://localhost:5000/getInstagram", "http://localhost:5000/getUserInstagram/tbg", "http://localhost:5000/getUserInstagram/webbys"};
//	vector<string> urls = {"http://localhost:5000/getUserInstagram/tbg"};
	vector<string> urls = {"http://localhost:5000/getBoothPhotos"};
	
	//	mPhotoStream = make_shared<PhotoStream>( "http://localhost/RoofiesWebbys/photos/photos.json" ); // test url for running without pythong scripts running
	mPhotoStream = make_shared<PhotoStream>( urls );
	
	// Create the grid of photos
//	float spacing = ( getWindowHeight() - ( size.x * rows)) / rows;
	float spacing = 0.0;
	float startX = ( getWindowWidth() -  ( ( size.x * cols) + ( (cols - 1) * spacing ) ) ) / 2;
	float startY = ( getWindowHeight() - ( ( size.y * rows) + ( (rows - 1) * spacing ) ) ) / 2;
	for( int i = 0; i < amt; i ++ ){
		float spacingX = ( i % cols == 0) ? 0 : spacing;
		float spacingY = ( i < cols ) ? 0 : spacing;
		GridPhoto* photo = new GridPhoto( Vec2f( startX + ( ( i % cols ) * ( size.x + spacingX ) ),
												startY + ( ( floor( i / cols ) * ( size.y + spacingY ) ) ) ),
										 size,
										 mSquareShadow,
										 mCam);
		gridPhotos.push_back( photo );
	}
	
	// start the timer that looks for new photos
	mLoadPhotoTimer = timeline().add( [&] { loadNewPhoto(); }, timeline().getCurrentTime() + 1.0  );
	mLoadPhotoTimer->setDuration( 0.2 );		// .2 seconds to start. Gets changed to less frequently later
	mLoadPhotoTimer->setAutoRemove( false );
	mLoadPhotoTimer->setLoop();
	
	mCam = CameraPersp();
	mEye        = Vec3f( getWindowWidth()/2, getWindowHeight()/2, 170.0f );
	mCenter     = Vec3f( getWindowWidth()/2, getWindowHeight()/2, 0 );
	mUp         = Vec3f::yAxis() * -1.0;
	
	mCam.lookAt( mEye, mCenter, mUp );
	
}

void ProjectionApp::resize( )
{
	// now tell our Camera that the window aspect ratio has changed
	mCam.setPerspective( 120.0f, -getWindowAspectRatio(), 5, 3000.0f );
	
}

void ProjectionApp::loadNewPhoto()
{
	//	console() << "LOAD NEW PHOTO" << endl;
	if(mLoadedPhotos.size() < 20 && mPhotoStream->hasPhotoAvailable())
	{
		// as long as there is are fewer than 20 photos stored and there are photos available, try to add more to the loaded photos vector
		PhotoRef photo = mPhotoStream->getNextPhoto();
		mLoadedPhotos.push_back( photo );
		
		// 15 photos is enough to get started
		if( mLoadedPhotos.size() >= 15 && !mStarted )
		{
			mLoadPhotoTimer->setDuration( 1.0 );
			setPhotos();
			mStarted = true;
			
			// start the timer that swaps out a photo for one of the grid items
			mSwapImageTimer = timeline().add( [&] { swapGridPhoto(); }, timeline().getCurrentTime() + 4.0  );
			mSwapImageTimer->setDuration( 4.0 );
			mSwapImageTimer->setAutoRemove( false );
			mSwapImageTimer->setLoop( true );
		}
	}
}


// Give each grid item a photo to display
void ProjectionApp::setPhotos()
{
	for( vector<GridPhoto*>::iterator g = gridPhotos.begin(); g != gridPhotos.end(); ++g ){
		if(mLoadedPhotos.size() == 0) continue;
		
		//		console() << "vector size" << mLoadedPhotos.size() << endl;
		(*g)->setPhoto( getRandomPhoto() );
	}
}

void ProjectionApp::swapGridPhoto()
{
	// pick a random grid item to set a new image for
	//	console() << "SWAP" << endl;
	GridPhoto *rand = gridPhotos[randInt(gridPhotos.size())];
	
	if(mLoadedPhotos.size() == 0) return;;
	rand->setPhoto( getRandomPhoto() );
}

// get a random PhotoRef from the list of loaded PhotoRefs
PhotoRef ProjectionApp::getRandomPhoto()
{
	int rand = randInt( mLoadedPhotos.size() );
	PhotoRef randPhoto = mLoadedPhotos[rand];
	mLoadedPhotos.erase( mLoadedPhotos.begin() + rand );
	return randPhoto;
}


void ProjectionApp::mouseDown( MouseEvent event )
{
}

void ProjectionApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' || event.getChar() == 'R' ){
		for( vector<GridPhoto*>::iterator g = gridPhotos.begin(); g != gridPhotos.end(); ++g ){
			(*g)->setTransition( GridPhoto::ROTATION );
		}
	}
	
	if( event.getChar() == 's' || event.getChar() == 'S' ){
		for( vector<GridPhoto*>::iterator g = gridPhotos.begin(); g != gridPhotos.end(); ++g ){
			(*g)->setTransition( GridPhoto::SLIDE );
		}
	}
}


void ProjectionApp::update()
{
	for( vector<GridPhoto*>::iterator g = gridPhotos.begin(); g != gridPhotos.end(); ++g ){
		(*g)->update();
	}
}

void ProjectionApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
//	gl::setMatricesWindow( getWindowWidth(), getWindowHeight(), true );
	
	gl::enableAlphaBlending();
	
//	gl::draw( mBackground );
	
//	gl::enable
//	gl::setMatrices( mCam );
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
	
	for( vector<GridPhoto*>::iterator g = gridPhotos.begin(); g != gridPhotos.end(); ++g ){
		(*g)->draw();
	}
//	gl::disableDepthRead();
//	gl::disableDepthWrite();
	
}

CINDER_APP_NATIVE( ProjectionApp, RendererGl )
