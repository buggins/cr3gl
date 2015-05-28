package org.coolreader.newui;

import java.util.Locale;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.speech.tts.TextToSpeech;

@SuppressWarnings("deprecation")
public class CRTTS implements TextToSpeech.OnInitListener, TextToSpeech.OnUtteranceCompletedListener {
	
	public interface TTSCallback {
	    void onTtsSentenceFinished();
	    void onTtsInitDone();
	}
	
	public static class VoiceInfo {
		public String id;
		public String name;
		public String lang;
		public VoiceInfo(String id, String name, String lang) {
			this.id = id;
			this.name = name;
			this.lang = lang;
		}
	}
	
	private class NewApiSupport {
		public void construct() {
			
		}
		public boolean setCurrentVoice(String id) {
			return false;
		}
	}
	
	@TargetApi(8)
	@SuppressLint("NewApi")
	private class Api8Support extends NewApiSupport {
		public void construct() {
			
		}
	}
	
	@TargetApi(18)
	@SuppressLint("NewApi")
	private class Api18Support extends NewApiSupport {
		public void construct() {
			
		}
	}
	
	@TargetApi(21)
	@SuppressLint("NewApi")
	private class Api21Support extends NewApiSupport {
		public void construct() {
			
		}
		public boolean setCurrentVoice(String id) {
			return false;
		}
	}
	
	private NewApiSupport _apiSupport; 

	public CRTTS(Context context) {
		_context = context;
		if (android.os.Build.VERSION.SDK_INT >= 21)
			_apiSupport = new Api21Support();
		else if (android.os.Build.VERSION.SDK_INT >= 18)
			_apiSupport = new Api18Support();
		else if (android.os.Build.VERSION.SDK_INT >= 8)
			_apiSupport = new Api8Support();
		else
			_apiSupport = new NewApiSupport();
		setDefaultVoiceList();
		_apiSupport.construct();
		create(null);
	}
	
	public void create(String engineName) {
		stop();
		if (_textToSpeech != null) {
			_textToSpeech.shutdown();
			_textToSpeech = null;
		}
		log.i("CRTTS.create " + engineName);
		_engineName = engineName;
		_initialized = false;
		_error = false;
		_textToSpeech = new TextToSpeech(_context, this, _engineName);
	}

	@Override
	public void onInit(int status) {
		_initialized = status == TextToSpeech.SUCCESS;
		_error = (status != TextToSpeech.SUCCESS);
		log.i("CRTTS.onInit status = " + (status == TextToSpeech.SUCCESS ? "SUCCESS" : "ERROR"));
		if (_initialized) {
			_textToSpeech.setOnUtteranceCompletedListener(this);
			_rate = 50;
			Locale locale = _textToSpeech.getLanguage();
			String lang = locale.getLanguage();
			log.i("TTS language: " + lang);
			if (_ttsCallback != null)
				_ttsCallback.onTtsInitDone();
		}
	}

	@Override
	public void onUtteranceCompleted(String utteranceId) {
		if (_ttsCallback != null)
			_ttsCallback.onTtsSentenceFinished();
		_speaking = false;
	}
	
	public TTSCallback getTextToSpeechCallback() {
		return _ttsCallback;
	}

	public void setTextToSpeechCallback(TTSCallback callback) {
    	_ttsCallback = callback;
    }
	
    public VoiceInfo[] getAvailableVoices() {
    	return _voices;
    }

    public VoiceInfo getCurrentVoice() {
    	return _currentVoice;
    }

    public VoiceInfo getDefaultVoice() {
    	return _defaultVoice;
    }

    public boolean setCurrentVoice(String id) {
		if (_textToSpeech == null)
			return false;
    	return _apiSupport.setCurrentVoice(id);
    }

    public boolean setRate(int rate) {
		if (_textToSpeech == null)
			return false;
		if (rate < 0)
			rate = 0;
		if (rate > 100)
			rate = 100;
		_textToSpeech.setSpeechRate(rate + 50 / 100.0f);
		_rate = rate;
    	return false;
    }

    public int getRate() {
		if (_textToSpeech == null)
			return 50;
    	return _rate;
    }

    public boolean canChangeCurrentVoice() {
		if (_textToSpeech == null)
			return false;
    	return false;
    }

    public boolean tell(String text) {
		if (_textToSpeech == null)
			return false;
    	return false;
    }

    public boolean isSpeaking() {
    	return _speaking;
    }

    public void stop() {
		if (_textToSpeech != null) {
			_textToSpeech.stop();
			_speaking = false;
		}
    }

    public boolean isInitialized() {
    	return _initialized;
    }
    
    public boolean isError() {
    	return _error;
    }
    
    public void uninit() {
    	if (_textToSpeech != null)
    		_textToSpeech.shutdown();
    	_textToSpeech = null;
    	_initialized = false;
    	_speaking = false;
    }
    
    private void setDefaultVoiceList() {
    	_voices = new VoiceInfo[1];
    	_voices[0] = new VoiceInfo(null, "System Default", "unknown language");
    	_currentVoice = _voices[0];
    	_defaultVoice = _voices[0];
    }
    
	public static final String TAG = "cr3tts";
	public static final Logger log = L.create(TAG);
	
	private Context _context;
	private TextToSpeech _textToSpeech;
	private String _engineName;
	private boolean _initialized;
	private boolean _error;
	private boolean _speaking;
	private TTSCallback _ttsCallback;
	private VoiceInfo[] _voices;
	private VoiceInfo _currentVoice;
	private VoiceInfo _defaultVoice;
	private int _rate;
}
