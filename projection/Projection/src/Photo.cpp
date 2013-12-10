//
//  Photo.cpp
//  IvyWall
//
//  Created by Greg Kepler on 11/27/12.
//
//

#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Photo.h"

//#include "cinder/app/AppBasic.h

using namespace ci;
using namespace ci::app;
using namespace std;

const static float GIF_FPS = 2.0;

Photo::Photo( const std::string &imageUrl, const ci::Surface &image )
:mUser( "" ), mImageUrl( imageUrl ), mDescription( "" ), mImage( image ), mGifFrame(0), mImageType(STATIC), mLoaded( false )
{
	initialize();
}

Photo::Photo( const std::string &imageUrl, const std::vector<ci::Surface> &mGifImages )
:mUser( "" ), mImageUrl( imageUrl ), mDescription( ""), mGifImages( mGifImages ), mGifFrame(0), mImageType(ANIMATED), mLoaded( false )
{
	mTotalFrames = mGifImages.size();
	mConvertedFrames = 0;
	initialize();
}


Photo::Photo( const std::string &user, const std::string &imageUrl, const std::string description, const ci::Surface &image )
:mUser( user ), mImageUrl( imageUrl ), mDescription( description), mImage( image ),
mGifFrame(0), mImageType(STATIC), mLoaded( false )
{
	initialize();
}

Photo::Photo( const std::string &user, const std::string &imageUrl, const std::string description, const std::vector<ci::Surface> &mGifImages )
:mUser( user ), mImageUrl( imageUrl ), mDescription( description), mGifImages( mGifImages ),
mGifFrame(0), mImageType(ANIMATED), mLoaded( false )
{
	mTotalFrames = mGifImages.size();
	mConvertedFrames = 0;
	initialize();
}

void Photo::initialize()
{
	mScale = randFloat( 350.0, 400.0 );
	mDrawReady = false;
	mTexFmt.enableMipmapping( true );
	mTexFmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
}

Photo::~Photo()
{
	mGifImagesTex.clear();
	console() << "photo killed" << endl;
}

bool Photo::isLoaded()
{
	return checkLoaded();
}

void Photo::setAdded( gl::GlslProg shader, gl::Texture frameTex )
{
	mShader = shader;
	mFrame = frameTex;
}

void Photo::setForDisplay( Vec2f loc, float rot )
{
	mLocation = loc;
	mRotation = rot;
	mDrawReady = true;	// can't be draw ready until it's loaded
}

bool Photo::isDrawReady()
{
	return mDrawReady;
}

gl::Texture Photo::getNextImage()
{
	if( mImageType == ANIMATED )
	{
		if( int(getElapsedFrames()) % int( getFrameRate() / GIF_FPS ) == 0)
		{
			mGifFrame++;
			if(mGifFrame > mGifImagesTex.size() - 1){
				mGifFrame = 0;
			}
		}
		
		// FIX: returns an error here sometimes - mGifImagesTex size = 0, mLoaded = true
		mImageTex = mGifImagesTex[mGifFrame];
		//		return mGifImagesTex[mGifFrame];
	}else{
		//		return mImage;
		//		return mImageTex;
	}
	return mImageTex;
}

gl::Texture Photo::getCurImage()
{
	return mImageTex;
}



void Photo::update()
{
	//console() << "update: " << mImage << endl;
	checkLoaded();
}


bool Photo::checkLoaded()
{
	if(mLoaded) return true;
	
	
	//bool loaded = false;
	if( mImageType == ANIMATED )
	{
		if( mGifImages[mGifFrame] )
		{
			/*
			 // loop through and make all surfaces textures
			 for(int i = 0; i < mGifImages.size(); i++ )
			 {
			 mGifImagesTex.push_back( gl::Texture( mGifImages[i] ) );
			 }
			 mLoaded = true;
			 */
			
			if(getElapsedFrames() % 15 == 0){
				// add 1 gif at a time
				gl::Texture tex = gl::Texture( mGifImages[mConvertedFrames], mTexFmt );
				
				mAspectRatio = tex.getAspectRatio();
				//				mGifImagesTex.push_back( applyShader( mGifImages[mConvertedFrames] ) );
				mGifImagesTex.push_back( tex );
				
				//mGifImagesTex.push_back( applyShader( tex ) );
				mConvertedFrames++;
				
				if(mConvertedFrames >= mGifImages.size()){
					//					mAspectRatio = mGifImages[0].getAspectRatio();
					mImageTex = mGifImagesTex[0];
					//mGifImages[0].getAspectRatio();
					// It's all loaded
					mLoaded = true;
				}else{
					//console() << "RETURN CONVERT" << endl;
					return mLoaded;
				}
			}else{
				return false;
			}
			
		}
	}else{
		
		if( mImage )
		{
			// make a texture from mImage
			//			mImageTex = gl::Texture( mImage, mTexFmt );
			gl::Texture tex = gl::Texture( mImage, mTexFmt );
			
			
			mAspectRatio = mImage.getAspectRatio();
			//			mImageTex = applyShader( mImage );
			mImageTex = tex;
			
			mLoaded = true;
			//loaded = true;
		}
	}
	mGifImages.clear();
	
	//mDropShadow = gl::Texture( loadImage( loadResource( "square_shadow.png" ) ) );
	//console() << "Return loaded" << endl;
	return mLoaded;
}


//gl::Texture Photo::applyShader( gl::Texture photo )
gl::Texture Photo::applyShader( Surface photo )
{
	Area viewport = gl::getViewport();
	gl::Fbo mFboScene = gl::Fbo( mScale * mAspectRatio, mScale );
	
	// *** SHADER STUFF ***
	gl::setViewport( mFboScene.getBounds() );
	mFboScene.bindFramebuffer();
	gl::pushMatrices();
	gl::setMatricesWindow( mScale * mAspectRatio, mScale, false);
	gl::clear( Color::white() );
	
	
	mShader.bind();
	mShader.uniform("tex0", 0);
	mShader.uniform("hue", Vec3f(0.0f, 0.0f, 0.0f) );
	mShader.uniform("alpha", 1.0f);
	mShader.uniform("brightness", 0.9f);
	mShader.uniform("saturation", 1.2f);
	mShader.uniform("contrast", 1.2f);
	gl::enable(GL_TEXTURE_2D);
	
	//	photo.bind(0);
	gl::draw( photo, Rectf( 0.0f, 0.0f, mScale * mAspectRatio, mScale));
	//	gl::drawSolidRect( Rectf( mFboScene.getBounds() ), false);
	//	gl::drawSolidRect( Rectf( 0.0f, 0.0f, 472.0f, 472.0f), false);
	//	photo.unbind();
	mShader.unbind();
	
	
	gl::popMatrices();
	mFboScene.unbindFramebuffer();
	
	// ******
	gl::setViewport( viewport );
	
	gl::Texture returnTex = mFboScene.getTexture();
	return returnTex;
}






void Photo::draw( const ci::Vec2f &imageSize )
{
	if( !mLoaded ) return;
	
	//gl::Texture photo = gl::Texture( getImage() );
	gl::Texture photo = getNextImage();
	
	glPushMatrix();
	//	gl::translate(mLocation);
	//	gl::rotate( mRotation );
	
	
	float scaleFactor = 0.5f;
	Rectf imageRect = Rectf( 0, 0, imageSize.x, imageSize.y );
	gl::draw( photo, imageRect );
	
	glPopMatrix();
	
}