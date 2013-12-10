//
//  GridPhoto.cpp
//  WebbysProjection
//
//  Created by Greg Kepler on 7/30/13.
//
//

#include "cinder/app/AppBasic.h"
#include "GridPhoto.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"

using namespace std;
using namespace ci::app;
using namespace ci;

GridPhoto::GridPhoto( Vec2f pos, Vec2f size, gl::Texture bg, ci::CameraPersp cam )
:mPos( pos ), mSize( size ), mBgShadow( bg ), mCam( cam )
{
	mAlpha = 0;
	mAnimationType = SLIDE;
//	mAnimationType = ROTATION;
}

void GridPhoto::setPhoto( PhotoRef photo )
{
	mQueuedPhoto = photo;
	mLoading = true;
	
	mTransitionPos = mPos + Vec2f(0.0, 0);
}

void GridPhoto::setTransition( Animation type )
{
	mAnimationType = type;
}

void GridPhoto::update()
{
	// check to see if the QueuedPhoto is loaded
	if( mLoading && mQueuedPhoto && mQueuedPhoto->isLoaded() ){
		mLoading = false;
		
		if( mAnimationType == SLIDE ){
			mAnimationDir = randInt(4);
	//		mAnimationDir = 3;
			switch( mAnimationDir ){
				case 0:
					mTransitionPos = Vec2f(-1.0, 0.0);
					break;
				case 1:
					mTransitionPos = Vec2f(1.0, 0.0);
					break;
				case 2:
					mTransitionPos = Vec2f(0.0, -1.0);
					break;
				case 3:
					mTransitionPos = Vec2f(0.0, 1.0);
					break;
			}
			timeline().apply( &mTransitionPos, Vec2f(0.0, 0.0), 1.00, EaseInOutBack() ).finishFn( bind( &GridPhoto::onCompleteTransition, this) );
		}
		else if( mAnimationType == ROTATION )
		{
			mAnimationDir = randInt(2);
			switch( mAnimationDir ){
				case 0:
					mRot = 360.0;
					break;
				case 1:
					mRot = -360.0;
					break;
			}
			
			
			mRotTween = timeline().apply( &mRot, float(0.0), 2.00, EaseInOutBack() ).finishFn( bind( &GridPhoto::onCompleteTransition, this) );
		}
		mTransitioning = true;
	}
	
	if( mQueuedPhoto ) {
		mQueuedPhoto->update();
		if(mQueuedPhoto->isLoaded()) mQueuedPhotoTex = mQueuedPhoto->getCurImage();
	}
	if( mCurPhoto ) {
		mCurPhoto->update();
		if(mCurPhoto->isLoaded()) mCurPhotoTex = mCurPhoto->getNextImage();
	}
}


void GridPhoto::draw()
{
	ColorA col;
	
	col = ColorA(1, 1, 1, 1.0);
	gl::color( col );
	
	gl::Texture drawTex;
	
	ci::Vec3f mRight, mUp;
	mCam.getBillboardVectors(&mRight, &mUp);
	
	glPushMatrix();
	
		// new photo
		gl::translate( mPos.x, mPos.y );
		
		glPushMatrix();
		gl::scale( mSize / mBgShadow.getSize() );
		gl::draw( mBgShadow );
		glPopMatrix();
	
		// transition old photo out and new photo in
		if(mTransitioning)
		{
			if( mAnimationType == ROTATION ){
				animateRotation();
			} else if( mAnimationType == SLIDE ){
				gl::scale( mSize );
				animateSlide();
			}
		}else{
			if( mCurPhotoTex && mCurPhoto && mCurPhoto->isLoaded() )
			{
				gl::scale( mSize );
				mCurPhotoTex.enableAndBind();
				gl::drawSolidRect( Rectf(0, 0, 1.0, 1.0), true );
				mCurPhotoTex.unbind();
				
				
			}
		}
	glPopMatrix();
	gl::color( ColorA(1, 1, 1, 1.0) );
//	gl::disableDepthRead();
//	gl::disableDepthWrite();
}


void GridPhoto::animateRotation()
{
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
	ColorA col1 = ColorA( 1, 1, 1, 1);
	ColorA col2 = ColorA( 1, 1, 1, 1);
	Vec3f newRot;
	if(mRotTween){
		float per = abs(mRot.value() / (mRotTween->getEndValue() - mRotTween->getStartValue()));
		switch(mAnimationDir){
			case 0:
				newRot = Vec3f( mRot.value(), 0, 0 );
				break;
				
			case 1:
				newRot = Vec3f( 0, mRot.value(), 0 );
				break;
		}
//		float per = abs(mRot.value() / (mRotTween->getEndValue() - mRotTween->getStartValue()));
		float alph1 = (per <= .25) ? 1.0 : 0;
		float alph2 = (per > .25) ? 1.0 : 0;
		col1 = ColorA( 1, 1, 1, alph1);
		col2 = ColorA( 1, 1, 1, alph2);
	}
	
	// new photo in
	if(mQueuedPhotoTex){
		glPushMatrix();
		
			gl::translate( mSize.x/2.0, mSize.y/2.0, 0.1 );
			gl::rotate( newRot );
			gl::scale( mSize );
			gl::translate( -0.5, -0.5, 0.0 );
					
			gl::color( col1 );
			mQueuedPhotoTex.enableAndBind();
			gl::drawSolidRect( Rectf(0.0, 0.0, 1.0, 1.0), true );
			mQueuedPhotoTex.unbind();
		glPopMatrix();
	}
	
	// old photo out
	if(mCurPhotoTex){
		glPushMatrix();
			gl::translate( mSize.x/2.0, mSize.y/2.0, 0.2 );
//		gl::translate( 0.5, 0.5, 0.1 );
			gl::rotate( newRot );
			gl::scale( mSize );
			gl::translate( -0.5, -0.5, 0.0 );
			
			gl::color( col2 );
			mCurPhotoTex.enableAndBind();
//		gl::drawBillboard( Vec3f::zero(), Vec2f(1.0, 1.0f), 0.0f, Vec3f(1, 0, 0), Vec3f(0, -1, 0));
//		gl::drawBillboard( Vec3f::zero(), Vec2f(1.0, 1.0f), 0.0f, Vec3f(1, 0, 0), Vec3f(0, -1, 0));
			gl::drawSolidRect( Rectf(0.0, 0.0, 1.0, 1.0), true );
//		gl::drawSolidRect( Rectf(-.5, -.5, .5, .5), true );
			mCurPhotoTex.unbind();
		
	
		
//		gl::color( Color8u::gray(127) );
//		gl::drawSolidRect( Rectf(0.0, 0.0, 1.0, 1.0), true );
		
		
		glPopMatrix();
	}
}

void GridPhoto::animateSlide()
{
	Rectf newRect;
	if( mQueuedPhotoTex ){
		
		// transition new photo (queued photo) in
		glPushMatrix();
		
		mQueuedPhotoTex.enableAndBind();
		switch( mAnimationDir ){
			case 0:
				gl::translate( mTransitionPos.value().x, 0.0 );
				newRect = getTextureRect( -mTransitionPos.value().x, 0, 1.0 - mTransitionPos.value().x , 1.0 );
				break;
			case 1:
				gl::translate( mTransitionPos.value().x , 0.0 );
				newRect = getTextureRect( 0 - mTransitionPos.value().x, 0, 1.0 - mTransitionPos.value().x, 1.0 );
				break;
			case 2:
				gl::translate( 0.0, mTransitionPos.value().y );
				newRect = getTextureRect( 0, 0 - mTransitionPos.value().y, 1.0, 1.0 - mTransitionPos.value().y );
				break;
			case 3:
				gl::translate( 0.0, mTransitionPos.value().y );
				newRect = getTextureRect( 0, 0 - mTransitionPos.value().y, 1.0, 1.0 - mTransitionPos.value().y );
				break;
		}
		
		gl::drawSolidRect( newRect, true );
		mQueuedPhotoTex.unbind();
		gl::color( ColorA(1, 1, 1, 1.0) );
		glPopMatrix();
	}
	
	
	// transition old photo out
	if(mCurPhoto){
		glPushMatrix();
		
		mCurPhotoTex.enableAndBind();		
		switch( mAnimationDir ){
			case 0:
				gl::translate( mTransitionPos.value().x + 1, 0.0 );
				newRect = getTextureRect(  0, 0, 0.0 - mTransitionPos.value().x, 1.0 );
				break;
			case 1:
				gl::translate( mTransitionPos.value().x - 1, 0.0 );
				newRect = getTextureRect( 1 - mTransitionPos.value().x, 0, 2.0 - mTransitionPos.value().x, 1.0 );
				break;
			case 2:
				gl::translate( 0.0, mTransitionPos.value().y + 1 );
				newRect = getTextureRect( 0, -1.0 - mTransitionPos.value().y, 1.0, 0.0 - mTransitionPos.value().y );
				break;
			case 3:
				gl::translate( 0.0, mTransitionPos.value().y - 1 );
				newRect = getTextureRect( 0, 1 - mTransitionPos.value().y, 1.0, 2.0 - mTransitionPos.value().y );
				break;
		}
		
		gl::drawSolidRect( newRect, true );
		mCurPhotoTex.unbind();
		glPopMatrix();
	}

}

// get the rectangle based on the direction of the animation
Rectf GridPhoto::getTextureRect( float x1, float y1, float x2, float y2 )
{
	return Rectf( clamp( x1 ), clamp( y1 ), clamp( x2 ), clamp( y2 ) );
}

float GridPhoto::clamp( float val )
{
	if( val == 0.0 || val == 1.0 ){
		return val;
	}
	return math<float>::clamp( val, 0.0, 1.0 );
}

void GridPhoto::onCompleteTransition()
{
	if(mQueuedPhoto) {
		mCurPhoto = mQueuedPhoto;
		mCurPhotoTex = mCurPhoto->getCurImage();
		mQueuedPhoto = NULL;
		mTransitioning = false;
	}
}