//
//  PhotoStream.cpp
//  IvyWall
//
//  Created by Greg Kepler on 11/20/12.
//
//

#include "cinder/app/AppBasic.h"
#include "cinder/Filesystem.h"
#include "cinder/Json.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timeline.h"
#include "cinder/ip/Resize.h"

#include "PhotoStream.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int IMAGE_BUFFER = 4;

JsonTree queryPhotos( const std::string &query );
JsonTree queryFoursquare( const std::string &query );
Surface surfaceResized( const Surface &fullSize, float percent );

PhotoStream::PhotoStream( vector<string> urls )
:mBuffer( IMAGE_BUFFER ),
mPhotoJsonUrls( urls ),
mCanceled( false )
{
	// set up concurrent circ buffer to fill data in with (10 at a time)
	startThread();
}

PhotoStream::~PhotoStream()
{
	mCanceled = true;
	mBuffer.cancel();
	mPhotoThread.join();
}

void PhotoStream::startThread(){
	mCurUrl = 0;
	mPhotoThread = thread( bind( &PhotoStream::servicePhotos, this, mPhotoJsonUrls ) );
}



// Function the background thread lives in
void PhotoStream::servicePhotos( const vector<string> &urls )
{
	ThreadSetup threadSetup;
	string nextQueryString = urls[mCurUrl];
	
	JsonTree searchResults;
	JsonTree::ConstIter resultIt = searchResults.end();
	
	// This function loops until the app quits. Each iteration a pulls out the next result from the Twitter API query.
	// When it reaches the last result of the current query it issues a new one, based on the "refresh_url" property
	// of the current query.
	// The loop doesn't spin (max out the processor) because ConcurrentCircularBuffer.pushFront() non-busy-waits for a new
	// slot in the circular buffer to become available.
	JsonTree queryResult;
	
	while( ! mCanceled ) {
		if( resultIt == searchResults.end() ) {			// are we at the end of the results of this JSON query?
			// issue a new query
			try {
				
//				console () << "SERVICE PHOTOS" << endl;
				
				nextQueryString = urls[mCurUrl];
				mCurUrl++;
				if( mCurUrl >= urls.size() ) mCurUrl = 0;
				
				queryResult = queryPhotos( nextQueryString );
				console() << queryResult << endl;
				// the next query will be the "refresh_url" of this one.
				
				searchResults = queryResult.getChild("photos");
				
				resultIt = searchResults.begin();
				mIsConnected = true;
			}
			//			catch( const runtime_error& error ) {
			catch( ... ) {
				
				//				console() << error.what() << endl;
				console() << "something broke" << endl;
				/*console() << queryResult << endl;
				 console() << nextQueryString << endl;
				 
				 // check if it's a 420 error
				 if(queryResult.getChild("meta").getChild("code").getValue() == "420"){
				 console() << "420 error" << endl
				 ;
				 mIsConnected = false;
				 }
				 */
				ci::sleep( 2000 ); // try again in 1 second
			}
		}
		if( resultIt != searchResults.end() ) {
			try {
				
				//				string userName = (*resultIt).getChild("user").getValue();
				//				string description = (*resultIt).getChild("description").getValue();
				
				string imageUrl = (*resultIt).getChild("url").getValue();
				string imageType = imageUrl.substr(imageUrl.find_last_of(".") + 1);
				
				
				// set the url depending on if it's via http or local disk
				DataSourceRef imagePath;
				if( imageUrl.find("http") != std::string::npos){	// If the image is a remote file
					imagePath = loadUrl( imageUrl );
				}else{
					imagePath = loadFile( imageUrl );				// If the image is a local file
				}
				
				bool success = true;
								
				if( imageType == "gif" )
				{
					vector<ImageSource::Options> imageOptions;
					
					ImageSource::Options options;
					std::vector<ci::Surface> gifImages;
					
					try
					{
						options.index( 0 );
						
						while ( 1 )
						{
							// add the url to the mutex of urls to load
							// start the loading
							string url = imageUrl + "?" + toString( time(NULL) );
							
							// Add teh surface to the array
							Surface gifImage( loadImage( imagePath, options ) );
							gifImages.push_back( gifImage );
							
							//mAnimGifParts.push_back( loadImage( loadResource( "conan.gif" ), options ) );
							options.index( options.getIndex() + 1 );
						}
					}
					catch( const ImageIoExceptionFailedLoad &e )
					{
						console() << "Error loading image/frame: " << imageUrl << " at frame " << options.getIndex() << endl;
						
						// if it can't load frame 0, let's try 3 times (1 seconds in between)
						if(options.getIndex() == 0){
							success = false;
							ci::sleep(1000);
						}
						
					}
					if(success){
						console() << " GOT THE GIF" << endl;
						// only do this if it succesfully foundthe gifs
						if(gifImages.size() > 1)
						{
							PhotoRef photoPtr = PhotoRef( new Photo( imageUrl, gifImages ) );
							mBuffer.pushFront( photoPtr );
						}
						else
						{
							//							PhotoRef photoPtr = PhotoRef( new Photo( userName, imageUrl, description, gifImages[0] ) );
							PhotoRef photoPtr = PhotoRef( new Photo( imageUrl, gifImages[0] ) );
							mBuffer.pushFront( photoPtr );
							
						}
					}
				}else { // not a gif
					
					try{
						//					Surface surfPhoto( loadImage( loadUrl( imageUrl ) ) );
						//gl::Texture photo = gl::Texture( surfPhoto );
						string url = imageUrl + "?" + toString( time(NULL) );
//						console() << "url: " << url << endl;
						
						Surface photo( loadImage( imagePath ) );
						//Surface photo = surfaceResized( orig, 0.8f );
						
						//						PhotoRef photoPtr = PhotoRef( new Photo( userName, imageUrl, description, photo ) );
						PhotoRef photoPtr = PhotoRef( new Photo( imageUrl, photo ) );
						mBuffer.pushFront( photoPtr );
					}
					catch( ... )
					{
						console() << "NO DICE" << endl;
						ci::sleep(1000);
					}
				}
				
				// try and load more than 1 frame
				// if it's only 1 frame, add as a 1 frame photo
				// else load a vector of photos
			}
			catch( const ImageIoExceptionFailedLoad &e ){
				console() << "ERRORS FOUND: " << e.what() << endl;
				
			}
			++resultIt;
		}
	}
	
}



JsonTree queryPhotos( const std::string &searchUrl )
{
	try{
		string fullUrl = searchUrl + "?" + toString( time(NULL) );	// append time to end of url to guard against caching
//		console() << "full url :" << fullUrl << endl;
		Url url( searchUrl , true );
		return JsonTree( loadUrl( fullUrl ) );
	}
	catch(...)
	{
		return( JsonTree() );
	}
}


bool PhotoStream::hasPhotoAvailable()
{
	return mBuffer.isNotEmpty();
}

PhotoRef PhotoStream::getNextPhoto()
{
	PhotoRef result;
    mBuffer.popBack( &result );
	//console() << "NEXT PHOTO: " << result->getImageUrl() << endl;
	return result;
}

bool PhotoStream::isConnected()
{
	return mIsConnected;
}


Surface surfaceResized( const Surface &fullSize, float percent ) {
	
    Surface result( fullSize.getWidth() * percent, fullSize.getHeight() * percent, fullSize.hasAlpha(), fullSize.getChannelOrder() );
	
    ip::resize( fullSize, &result );
	
    return result;
	
}



