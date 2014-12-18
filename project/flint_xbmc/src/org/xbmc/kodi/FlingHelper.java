package org.xbmc.kodi;

import java.io.IOException;
import java.util.ArrayList;

import org.json.JSONObject;

import tv.matchstick.flint.ApplicationMetadata;
import tv.matchstick.flint.ConnectionResult;
import tv.matchstick.flint.Flint;
import tv.matchstick.flint.FlintDevice;
import tv.matchstick.flint.FlintManager;
import tv.matchstick.flint.FlintMediaControlIntent;
import tv.matchstick.flint.MediaInfo;
import tv.matchstick.flint.MediaMetadata;
import tv.matchstick.flint.MediaStatus;
import tv.matchstick.flint.RemoteMediaPlayer;
import tv.matchstick.flint.ResultCallback;
import tv.matchstick.flint.Status;
import tv.matchstick.flint.Flint.ApplicationConnectionResult;
import tv.matchstick.flint.RemoteMediaPlayer.MediaChannelResult;
import android.content.Context;
import android.os.Bundle;
import android.support.v7.media.MediaRouteSelector;
import android.support.v7.media.MediaRouter;
import android.support.v7.media.MediaRouter.RouteInfo;
import android.util.Log;

public class FlingHelper {
	
	private static final String TAG = "FlingHelper";
	
	protected String mApplicationId;
	protected String mAppUrl;
	protected MediaRouter mMediaRouter;
	protected MediaRouteSelector mMediaRouteSelector;
    protected FlingMediaRouterCallback mMediaRouterCallback;
    
    protected FlintDevice mSelectedDevice;
    protected FlintManager mApiClient;
    protected boolean mConnectionSuspened;
    protected boolean mApplicationStarted = false;
    private RemoteMediaPlayer mRemoteMediaPlayer;
    private int mState = MediaStatus.PLAYER_STATE_IDLE;
    private int mIdleReason;
    
    protected Context mContext;
    public static IRemotePlayController mRemotePlayController;
    private ArrayList<RouteInfo> mRouteList = new ArrayList<RouteInfo>();
    
    protected FlingHelper(Context context) {
    	mContext = context;
    	
    	mMediaRouter = MediaRouter.getInstance(mContext);
		mMediaRouterCallback = new FlingMediaRouterCallback();
		
		mApplicationId = "~flintplayer";
		mAppUrl = "http://openflint.github.io/flint-player/player.html";
		Flint.FlintApi.setApplicationId(mApplicationId);
		mMediaRouteSelector = new MediaRouteSelector.Builder().addControlCategory(
                FlintMediaControlIntent.categoryForFlint(mApplicationId)).build();
    	
    }
    
    public void addMediaRouterCallback() {
    	mMediaRouter.addCallback(mMediaRouteSelector, mMediaRouterCallback,
                MediaRouter.CALLBACK_FLAG_PERFORM_ACTIVE_SCAN);
    }
    
    public void removeMediaRouterCallback() {
    	mMediaRouter.removeCallback(mMediaRouterCallback);
    }
    
    public void setDevice(FlintDevice device, boolean stopAppOnExit) {
		mSelectedDevice = device;
		
        if (null == mApiClient) {
        	Log.d(TAG, "acquiring a connection to Matchstick Fling services for " + mSelectedDevice);
            Flint.FlintOptions.Builder apiOptionsBuilder = Flint.FlintOptions
            		.builder(mSelectedDevice, new FlintListener());
            mApiClient = new FlintManager.Builder(mContext)
                    .addApi(Flint.API, apiOptionsBuilder.build())
                    .addConnectionCallbacks(new ConnectionCallbacks())
                    .build();
            mApiClient.connect();
        } else if (!mApiClient.isConnected()) {
            mApiClient.connect();
        }
    }
    
    public void addRoute(RouteInfo route) {
    	boolean bExists=false;
        for(RouteInfo routeInfo:mRouteList){
        	if (routeInfo.getId()==route.getId()){
        		bExists=true;
        		break;
        	}
        }
        if (!bExists){
        	mRouteList.add(route);
        }
    }
    
    public void selectRoute(RouteInfo route){
    	Log.d(TAG, "selectRoute route:"+route.getName());
    	mMediaRouter.selectRoute(route);
    }
    
    public ArrayList<RouteInfo> getRouteList() {
    	return mRouteList;
    }
    
    public void removeRoute(RouteInfo route){
    	for(RouteInfo routeInfo:mRouteList){
        	if (routeInfo.getId()==route.getId()){
        		mRouteList.remove(routeInfo);
        		break;
        	}
        }
    }
    
    public void disconnect() {
    	teardown();
    }
    
    public void teardown() {
    	Log.d(TAG, "teardown");
    	mState = MediaStatus.PLAYER_STATE_IDLE;
    	if (mApiClient != null) {
			if (mApiClient.isConnected() || mApiClient.isConnecting()) {
				try {
					Flint.FlintApi.stopApplication(mApiClient).setResultCallback(new ResultCallback<Status>() {

			            @Override
			            public void onResult(Status result) {
			                if (!result.isSuccess()) {
			                    Log.d(TAG, "stopApplication -> onResult: stopping " + "application failed");
			                } else {
			                    Log.d(TAG, "stopApplication -> onResult Stopped application " + "successfully");
			                }
			            }
			        });
					
				} catch (Exception e) {
					Log.e(TAG, "Exception while removing channel", e);
				}
				
				try {
					if (mRemoteMediaPlayer != null) {
						Flint.FlintApi.removeMessageReceivedCallbacks(
								mApiClient,
								mRemoteMediaPlayer.getNamespace());
						mRemoteMediaPlayer = null;
					}
				} catch (IOException e) {
					Log.e(TAG, "IOException while removing channel", e);
				} catch (Exception e) {
					Log.e(TAG, "Exception while removing channel", e);
				}
				
				mApiClient.disconnect();
			}
			if (null != mMediaRouter && null != mMediaRouter.getDefaultRoute()) {
				Log.e(TAG, "select default route");
                mMediaRouter.selectRoute(mMediaRouter.getDefaultRoute());
            }
    		mApiClient = null;
    	}
    	mSelectedDevice = null;
    	mConnectionSuspened = false;
  	}
    
    void onApplicationConnected(ApplicationMetadata appMetadata,
            String applicationStatus, boolean wasLaunched) {
    	attachMediaChannel();
    	mRemoteMediaPlayer.requestStatus(mApiClient).
        setResultCallback(new ResultCallback<RemoteMediaPlayer.MediaChannelResult>() {

            @Override
            public void onResult(MediaChannelResult result) {
                if (!result.getStatus().isSuccess()) {
                    Log.e(TAG, "Failed to request status.");
                }

            }
        });
    }
    
    private void attachMediaChannel() {
    	Log.d(TAG, "attachMedia()");
		if (null == mRemoteMediaPlayer) {
		    mRemoteMediaPlayer = new RemoteMediaPlayer();
		
		    mRemoteMediaPlayer.setOnStatusUpdatedListener(
		            new RemoteMediaPlayer.OnStatusUpdatedListener() {
		                @Override
		                public void onStatusUpdated() {
		                    Log.d(TAG, "RemoteMediaPlayer::onStatusUpdated() is reached");
		                    mState = mRemoteMediaPlayer.getMediaStatus().getPlayerState();
		                    mIdleReason = mRemoteMediaPlayer.getMediaStatus().getIdleReason();
		                    mRemotePlayController.UpdatePlayStatus(mState);
		                }
		            });
		
		    mRemoteMediaPlayer.setOnMetadataUpdatedListener(
		            new RemoteMediaPlayer.OnMetadataUpdatedListener() {
		                @Override
		                public void onMetadataUpdated() {
		                    Log.d(TAG, "RemoteMediaPlayer::onMetadataUpdated() is reached");
		                }
		            });
		
		}
		try {
		    Log.d(TAG, "Registering MediaChannel namespace");
		    Flint.FlintApi.setMessageReceivedCallbacks(mApiClient, 
		    		mRemoteMediaPlayer.getNamespace(),
		            mRemoteMediaPlayer);
		} catch (Exception e) {
		    Log.e(TAG, "Failed to set up media channel", e);
		}
		
	}
    
    public void loadMedia(String title, String url, String mime) {
    	MediaMetadata mediaMetadata = new MediaMetadata(MediaMetadata.MEDIA_TYPE_MOVIE);
		mediaMetadata.putString(MediaMetadata.KEY_TITLE, title);
		MediaInfo mediaInfo = new MediaInfo.Builder(url)
			.setContentType(mime)
			.setStreamType(MediaInfo.STREAM_TYPE_BUFFERED)
			.setMetadata(mediaMetadata).build();
		try {
			mRemoteMediaPlayer.load(mApiClient, mediaInfo, true)
				.setResultCallback(new ResultCallback<RemoteMediaPlayer.MediaChannelResult>() {
					@Override
					public void onResult(MediaChannelResult result) {
						if (result.getStatus().isSuccess()) {
							Log.d(TAG, "Media loaded successfully");
						}
					}
			});
		} catch (IllegalStateException e) {
			Log.e(TAG, "Problem occurred with media during loading", e);
		} catch (Exception e) {
			Log.e(TAG, "Problem opening media during loading", e);
		}
    }
    
    public void togglePlayback() {
        if (mState == MediaStatus.PLAYER_STATE_PLAYING) {
            pause();
        } else if (mState == MediaStatus.PLAYER_STATE_PAUSED){
        	play();
        }
    }
    
    public void pause() {
    	pause(null);
    }
    
    public void pause(JSONObject customData) {
    	Log.d(TAG, "attempting to pause media");
		if (mRemoteMediaPlayer == null) {
		    Log.e(TAG, "Trying to pause a video with no active media session");
		    return;
		}
		mRemoteMediaPlayer.pause(mApiClient, customData)
		.setResultCallback(new ResultCallback<MediaChannelResult>() {
			
			@Override
            public void onResult(MediaChannelResult result) {
                if (!result.getStatus().isSuccess()) {
                	Log.e(TAG, "Pause video failed!");
                }
            }
		});
	}
    
    public void play() {
    	play(null);
    }
    
    public void play(JSONObject customData) {
    	Log.d(TAG, "play(customData)");
        if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "Trying to play a video with no active media session");
            return;
        }
        
        mRemoteMediaPlayer.play(mApiClient, customData)
        .setResultCallback(new ResultCallback<MediaChannelResult>() {

            @Override
            public void onResult(MediaChannelResult result) {
                if (!result.getStatus().isSuccess()) {
                	Log.e(TAG, "Play video failed!");
                }
            }

        });
    }
    
    public void play(int position) {
    	seekAndPlay(position);
    }
    
    public void seekAndPlay(int position) {
    	Log.d(TAG, "attempting to seek media");
        if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "Trying to seekAndPlay a video with no active media session");
            return;
        }
        
        ResultCallback<MediaChannelResult> resultCallback =
                new ResultCallback<MediaChannelResult>() {

                    @Override
                    public void onResult(MediaChannelResult result) {
                        if (!result.getStatus().isSuccess()) {
                        	Log.e(TAG, "Seek and play failed!");
                        }
                    }

                };
        mRemoteMediaPlayer.seek(mApiClient,
                position,
                RemoteMediaPlayer.RESUME_STATE_PLAY).setResultCallback(resultCallback);
    }
    
    public void incrementVolume(double delta) {
    	double vol = getVolume() + delta;
        if (vol > 1) {
            vol = 1;
        } else if (vol < 0) {
            vol = 0;
        }
        setVolume(vol);
    }
    
    public void setVolume(double volume) {
    	if (volume > 1.0) {
            volume = 1.0;
        } else if (volume < 0) {
            volume = 0.0;
        }
    	
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "Trying to setVolume with no active media session");
            return;
        }
    	
    	mRemoteMediaPlayer.setStreamVolume(mApiClient, volume).setResultCallback(
                new ResultCallback<RemoteMediaPlayer.MediaChannelResult>() {

                    @Override
                    public void onResult(MediaChannelResult result) {
                        if (!result.getStatus().isSuccess()) {
                        	Log.e(TAG, "Set volume failed!");
                        }
                    }
                }
        );
    }
    
    public double getVolume() {
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "No active media session");
            return 0.0;
        }
    	
    	return mRemoteMediaPlayer.getMediaStatus().getStreamVolume();
    }
    
    public boolean isMute() {
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "No active media session");
            return false;
        }
    	
    	return mRemoteMediaPlayer.getMediaStatus().isMute();
    }
    
    public MediaInfo getRemoteMediaInformation() {
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "No active media session");
            return null;
        }
    	
    	return mRemoteMediaPlayer.getMediaInfo();
    }
    
    public long getCurrentMediaPosition() {
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "No active media session");
            return 0;
        }
    	
    	return mRemoteMediaPlayer.getApproximateStreamPosition();
    }
    
    public long getMediaDuration() {
    	if (mRemoteMediaPlayer == null) {
            Log.e(TAG, "No active media session");
            return 0;
        }
    	
    	return mRemoteMediaPlayer.getStreamDuration();
    }
    
    public int getPlaybackStatus() {
        return mState;
    }
    
    
    
    private class FlingMediaRouterCallback extends MediaRouter.Callback {
        @Override
        public void onRouteAdded(MediaRouter router, RouteInfo route) {
            FlintDevice flintDevice =  FlintDevice.getFromBundle(route.getExtras());
            
            android.util.Log.d(TAG, "onRouteAdded: "
                    + flintDevice.getFriendlyName() + ":" + flintDevice.getIpAddress().getHostAddress());
            addRoute(route);
        }

        @Override
        public void onRouteRemoved(MediaRouter router, RouteInfo route) {
        	FlintDevice flintDevice = FlintDevice.getFromBundle(route.getExtras());
            android.util.Log.d(TAG, "onRouteRemoved: "
                    + flintDevice.getFriendlyName() + ":" + flintDevice.getIpAddress().getHostAddress());
            removeRoute(route);
        }
        
        @Override
        public void onRouteSelected(MediaRouter router, RouteInfo route) {
        	FlintDevice flintDevice =  FlintDevice.getFromBundle(route.getExtras());
            
            android.util.Log.d(TAG, "onRouteSelected: "
                    + flintDevice.getFriendlyName() + ":" + flintDevice.getIpAddress().getHostAddress());
            
            setDevice(flintDevice, false);
        }
        
        @Override
        public void onRouteUnselected(MediaRouter router, RouteInfo route) {
        	FlintDevice flintDevice =  FlintDevice.getFromBundle(route.getExtras());
            
            android.util.Log.d(TAG, "onRouteUnselected: "
                    + flintDevice.getFriendlyName() + ":" + flintDevice.getIpAddress().getHostAddress());

            teardown();
        }
    }
    
    private class FlintListener extends Flint.Listener {

        @Override
        public void onApplicationDisconnected(int statusCode) {
            Log.d(TAG, "onApplicationDisconnected: " + statusCode);
            teardown();
        }

        @Override
        public void onApplicationStatusChanged() {
        	if (mApiClient != null) {
        		Log.d(TAG, "onApplicationStatusChanged: " + Flint.FlintApi.getApplicationStatus(mApiClient));
        	}
        }

        @Override
        public void onVolumeChanged() {
        	if (mApiClient != null) {
        		Log.d(TAG, "onVolumeChanged: " + Flint.FlintApi.getVolume(mApiClient));
        	}
        }
    }
    
    private class ConnectionCallbacks implements FlintManager.ConnectionCallbacks {

		@Override
		public void onConnected(Bundle arg0) {
			Log.d(TAG, "onConnected() reached with prior suspension: " + mConnectionSuspened);
	        if (mConnectionSuspened) {
	            mConnectionSuspened = false;
	        }
			
	        try {
	            Flint.FlintApi.requestStatus(mApiClient);
	            Log.d(TAG, "Launching app");
	            Flint.FlintApi.launchApplication(mApiClient, mAppUrl).setResultCallback(
	                    new ResultCallback<Flint.ApplicationConnectionResult>() {

	                        @Override
	                        public void onResult(ApplicationConnectionResult result) {
	                            if (result.getStatus().isSuccess()) {
	                                Log.d(TAG, "launchApplication() -> success result");
	                                onApplicationConnected(result.getApplicationMetadata(),
	                                        result.getApplicationStatus(),
	                                        result.getWasLaunched());
	                            } else {
	                                Log.d(TAG, "launchApplication() -> failure result");
	                                teardown();
	                            }
	                        }
	                    }
	            );
	        } catch (IOException e) {
	            Log.e(TAG, "error requesting status", e);
	        } catch (IllegalStateException e) {
	            Log.e(TAG, "error requesting status", e);
	        }
		}

		@Override
		public void onConnectionSuspended(int cause) {
	        Log.d(TAG, "onConnectionSuspended() was called with cause: " + cause);
	        mConnectionSuspened = true;
		}

		@Override
		public void onConnectionFailed(ConnectionResult result) {
			// TODO Auto-generated method stub
			Log.d(TAG, "onConnectionFailed() reached, error code: " + result.getErrorCode()
	                + ", reason: " + result.toString());
			teardown();
		}
    	
    }
    
}
