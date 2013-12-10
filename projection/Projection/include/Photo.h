//
//  Photo.h
//  IvyWall
//
//  Created by Greg Kepler on 11/27/12.
//
//

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/ConcurrentCircularBuffer.h"
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

typedef std::shared_ptr<class Photo>		PhotoRef;

class Photo
: public std::enable_shared_from_this<Photo>
{
public:
	typedef enum { STATIC, ANIMATED } ImageType;
	
	Photo() {}
	Photo( const std::string &imageUrl, const ci::Surface &image );
	Photo( const std::string &imageUrl, const std::vector<ci::Surface> &mGifImages );
	Photo( const std::string &user, const std::string &imageUrl, const std::string description, const ci::Surface &image );
	Photo( const std::string &user, const std::string &imageUrl, const std::string description, const std::vector<ci::Surface> &mGifImages );
	//	virtual ~Photo();
	~Photo();
	
	void initialize();
	
	const std::string&		getUser()			const { return mUser; }
	const std::string&		getImageUrl()		const { return mImageUrl; }
	const std::string&		getDescription()	const { return mDescription; }
	ci::gl::Texture			getNextImage(), getCurImage();
	
	
	bool				isNull() const { return mImageUrl.empty(); }
	void				update();
	void				draw( const ci::Vec2f &imageSize );
	bool				isLoaded();
	bool				isDrawReady();
	bool				checkLoaded();
	void				addFrame( std::string url, ci::ImageSource::Options options);
	void				setAdded( ci::gl::GlslProg shader, ci::gl::Texture frameTex );
	void				setForDisplay( ci::Vec2f loc, float rot );
	
	ci::Vec2f			mLocation;
	float				mRotation;
	float				mScale;
	
	
private:
	std::string						mUser, mImageUrl, mDescription;
	ci::Surface						mImage;
	std::vector<ci::Surface>		mGifImages;
	ci::gl::Texture					mImageTex;
	std::vector<ci::gl::Texture>	mGifImagesTex;
	ci::gl::Texture					mDropShadow;
	ci::gl::Texture					mFrame;
	ci::gl::Texture::Format			mTexFmt;
	
	ImageType						mImageType;
	int								mGifFrame;
	bool							mLoaded, mDrawReady;	// There's a distinction between loaded and ready for drawing
	float							mAspectRatio;
	ci::gl::GlslProg				mShader;
	
	
	int								mTotalFrames, mConvertedFrames;
	
	//ci::gl::Texture					applyShader( ci::gl::Texture photo );
	ci::gl::Texture					applyShader( ci::Surface photo );
	
	GLfloat mCtrlPoints[4][4][3];
	
};