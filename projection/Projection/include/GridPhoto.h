//
//  GridPhoto.h
//  WebbysProjection
//
//  Created by Greg Kepler on 7/30/13.
//
//

#include "cinder/gl/Texture.h"
#include "cinder/Timeline.h"
#include "cinder/Camera.h"
#include "Photo.h"

class GridPhoto{
public:
	typedef enum { SLIDE, ROTATION } Animation;
	GridPhoto( ci::Vec2f pos, ci::Vec2f size, ci::gl::Texture bg, ci::CameraPersp cam );
	
	void update();
	void draw();
	void animateOut();
	void animateIn();
	void setPhoto( PhotoRef );
	void onCompleteTransition();
	void setTransition( Animation );
	
private:
	PhotoRef			mCurPhoto;
	PhotoRef			mQueuedPhoto;
	ci::gl::Texture		mCurPhotoTex, mQueuedPhotoTex;
	ci::gl::Texture		mBgShadow;
	
	ci::Vec2f			mPos;
	ci::Vec2f			mSize;
	int					mOrder;
	ci::Anim<ci::Vec2f>	mTransitionPos;		// position while it transitions in/out
	ci::Anim<float>	mRot;		// position while it transitions in/out
//	ci::Anim<ci::Vec3f>	mRot;		// position while it transitions in/out
	ci::TweenRef<float> mRotTween;
	
	float				mAlpha;
	bool				mLoading, mTransitioning;
	int					mAnimationDir;
	
	ci::Rectf			getTextureRect( float x1, float y1, float x2, float y2 );
	float				clamp( float val );
	
	ci::CameraPersp		mCam;
	Animation			mAnimationType;
	
	void				animateRotation();
	void				animateSlide();
	
	
};