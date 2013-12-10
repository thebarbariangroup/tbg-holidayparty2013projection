//
//  PhotoStream.h
//
//  Created by Greg Kepler on 7/30/12.
//
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/ConcurrentCircularBuffer.h"
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "Photo.h"
#include "cinder/Timeline.h"



class PhotoStream {
public:
	PhotoStream( std::vector<std::string> urls );
	~PhotoStream();
	
	// get next item
	PhotoRef				getNextPhoto();
	bool					isConnected();
	bool					hasPhotoAvailable();
	
private:
	
	std::vector<std::string>				mPhotoJsonUrls;
	std::thread								mPhotoThread;
	bool									mCanceled;
	bool									mIsConnected;
	int										mCurUrl, mUrlMax;	// urls for loading images
	
	ci::ConcurrentCircularBuffer< PhotoRef >	mBuffer;
	
	void		startThread( );
//	void		servicePhotos( std::string url );
	void		servicePhotos( const std::vector<std::string> &urls );
};