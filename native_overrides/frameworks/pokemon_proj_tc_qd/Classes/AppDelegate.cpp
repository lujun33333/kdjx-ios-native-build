/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "spine41/Spine41Lua.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"

#include "ymextra/crypto/CCCrypto.h"
#include "assets-manager/VersionUtils.h"
// #include "assets-manager/AssetsManager.h"
#include "ymextra/ymextra.h"
#include "assets-manager/AssetsManagerEx.h"
#include "platform/dump.h"

// #define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1
// #define SKIP_UPDATE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif


USING_NS_CC;
USING_NS_CC_EXT;
using namespace std;
using namespace cocos2d::network;

//////////////////// CONST_STRING BEGIN
const char _mapping[128] = {
	21,40,105,121,124,62,11,33,54,111,102,78,98,45,119,96,
	115,42,41,13,68,55,107,92,0,90,15,12,94,100,26,101,
	87,110,77,122,71,116,64,67,2,114,69,24,95,50,16,8,
	113,17,22,108,48,23,20,75,5,99,37,53,127,30,59,38,
	1,14,106,89,19,9,43,80,112,3,39,83,91,93,86,104,
	84,47,35,109,44,70,27,74,18,120,28,76,79,97,117,81,
	63,82,32,103,57,4,72,36,85,52,49,51,61,34,118,66,
	125,29,31,6,123,46,60,10,88,25,65,126,56,58,73,7
};


static std::string _mappingChars(const char* arr, int n)
{
	std::string ret;
	ret.resize(n);
	for (int i = 0; i < n; i++)
	{
		ret[i] = _mapping[arr[i]];
	}
	return ret;
}


// kCompanyShortName = "TianJi";
// _mappingChars(kCompanyShortName, sizeof(kCompanyShortName))
const char kCompanyShortName[6] = {80,2,93,33,87,2};

// kCompanyName = "TianJi Information Technology Inc.";
// _mappingChars(kCompanyName, sizeof(kCompanyName))
const char kCompanyName[34] = {80,2,93,33,87,2,98,126,33,10,9,41,83,93,37,2,9,33,98,80,31,57,79,33,9,51,9,99,3,98,126,33,57,117};
//////////////////// CONST_STRING END


static AssetsManagerEx* assetsManager = nullptr;



static unsigned int confuseKey(unsigned int v)
{
	unsigned int bitm = 0xf;
	for (int i = 0; i < 4; i++)
	{
		unsigned int hi = 4 * (7 - i);
		unsigned int lo = 4 * i;
		unsigned int hib = (v&(bitm << hi)) >> hi;
		unsigned int lob = (v&(bitm << lo)) >> lo;
		unsigned int mask = (bitm << hi) | (bitm << lo);
		v = (hib << lo) | (lob << hi) | ((v & mask) ^ v);
	}
	return v;
}

static char _toConfuse(char ch)
{
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 'A';
    else if (ch == '_') return '+';
    else if (ch == '.' || ch == '6') return ';';
    return ch;
}

static std::string _toLuaPwd(std::string version)
{
    // for code confusion
    // 1. a = _toConfuse("hello"+kCompanyShortName+",world"+kCompanyName)
    // 2. b = MD5(a) + string("20140516"+version+kCompanyShortName)
    // 3. c = _toConfuse(b)
    // 4. d = reverse(c)
    // 5. e = MD5(d)
    int pos = version.find_last_of('.'); //把最后一位舍掉了
    std::string confusion = "hello" + std::string(_mappingChars(kCompanyShortName, sizeof(kCompanyShortName))) + ",world" + _mappingChars(kCompanyName, sizeof(kCompanyName));
    std::transform(confusion.begin(), confusion.end(), confusion.begin(), _toConfuse);
    std::string updatePass = CCCrypto::MD5String((void*)confusion.c_str(), confusion.length());
    updatePass += "20140516" + version.substr(0, pos) + _mappingChars(kCompanyShortName, sizeof(kCompanyShortName));
    std::transform(updatePass.begin(), updatePass.end(), updatePass.begin(), _toConfuse);
    std::reverse(updatePass.begin(), updatePass.end());
    return CCCrypto::MD5String((void*)updatePass.c_str(), updatePass.length());
}


AppDelegate::AppDelegate()
{
    //VersionPlistInfo versionPlist = getLocalVersion();
    //std::string pwd = _toLuaPwd(versionPlist.app_version); //要取大版本号
    std::string pwd = _toLuaPwd("2.1.0.0");

    //pwd = "2da88ee346c7f6fdf194781ca593ecef"; // test with shuma key!!!
    // pwd    "d222827fe9d372a524908c6d0a96cba3"
    FileUtils::tjXXTEAKey(pwd.c_str(), pwd.length());
}

AppDelegate::~AppDelegate()
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif

#if (COCOS2D_DEBUG > 0) && (CC_CODE_IDE_DEBUG_SUPPORT > 0)
    // NOTE:Please don't remove this call if you want to debug with Cocos Code IDE
    RuntimeEngine::getInstance()->end();
#endif

}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0 };

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,
// don't modify or remove this function
// static int register_all_packages()
// {
//     return 0; //flag for packages manager
// }

bool AppDelegate::applicationDidFinishLaunching()
{
    // set default FPS
    Director::getInstance()->setAnimationInterval(1.0 / 60.0f);
    Director::getInstance()->setDisplayStats(false);

	//pvr.ccz的key
	// 37e2e2bc0953acedc06c1c75ff4fee3b
	// 0x37e2e2bc, 0x0953aced, 0xc06c1c75, 0xff4fee3b
	ZipUtils::setPvrCczKey(confuseKey(0xcb2e2e73), confuseKey(0xdeca3590), confuseKey(0x57c1c60c), confuseKey(0xb3eef4ff));

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
	// libbugrpt
	//LuaExceptionHandler::registerLuaExceptionHandler("A001968155");
#endif // #if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)

	// tianji crash reporter
	VersionPlistInfo versionPlist = getLocalVersion();
	InitClientDump(versionPlist.reportUrl);

#if CC_64BITS
	const char* arch = "x64";
#else
	const char* arch = "x86";
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	const char *platform = "win";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	const char *platform = "mac";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	const char *platform = "android";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	const char *platform = "ios";
#else
	const char *platform = "UNKNOWN";
#endif


	char buf[256];
	SetUserInfo("arch", arch);
	sprintf(buf, "%d", CC_TARGET_PLATFORM);
	SetUserInfo("platform_id", buf);
	SetUserInfo("platform", platform);
	SetUserInfo("version", versionPlist.app_version);
	sprintf(buf, "%d", versionPlist.patch);
	SetUserInfo("patch", buf);

	// update
	prepareUpdate();

	// test dump
// 	CCLOG("crash bengin!!!!");
// 	int* p = NULL;
// 	*p = 0;
// 	CCLOG("crash end!!!!");


#if SKIP_UPDATE
	CCLOG("update closed mode");
	// 延迟2帧
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
			this->updateSuccessCallBack();
		});
	});

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	FILE* noupdateFp = fopen(".noupdate", "rb");
	if (noupdateFp)
	{
		CCLOG("update closed mode in win32 with .noupdate");
		fclose(noupdateFp);
		// 延迟2帧
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
			Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
				this->updateSuccessCallBack();
			});
		});
	}
	else
	{
		didUpdateAndRun();
	}

#else
	didUpdateAndRun();
#endif
	return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}

// update flow:
// get remote version.conf            +------------+
//          +                         | updater.lua|
//          |                         +------------+
//          v     yes                 |            |
//       app old?+--->exit,           |     UI     |
//          +         update in store |            |
//          |                         |            |
//          v         no              |            |
//     version old?+------+           |            |
//          +             |           |            |
//          |             |           |            |
//          v             |  callback |            |
//  download and update+---------------->  55%     |
//          +             |           |            |
//          |             |           |            |
//          v             |           |            |
//  set path and reload   |           |            |
//          +             |           |            |
//          |             |           |            |
//          v             |           |            |
//      run game <--------+           +------------+
//
bool AppDelegate::didUpdateAndRun()
{
	// while (true)
	assetsManager->update();

	return true;
}

void AppDelegate::prepareUpdate()
{
	_initSearchPaths = FileUtils::getInstance()->getSearchPaths();

	std::string storagePath = FileUtils::getInstance()->getWritablePath() + "patch/";

	// 新项目方式，按diff server返回的文件列表来下载增量更新
	// http://wiki.tianji-game.com:8090/pages/viewpage.action?pageId=5013655
	assetsManager = AssetsManagerEx::create(CC_CALLBACK_0(AppDelegate::updateSuccessCallBack, this), storagePath);
	assetsManager->retain();
	// assetsManager->setConnectionTimeout(8);

	auto engine = LuaEngine::getInstance();
	ScriptEngineManager::getInstance()->setScriptEngine(engine);
	register_spine41(engine->getLuaStack()->getLuaState());

	//engine->getLuaStack()->setXXTEAKeyAndSign(pwd.c_str(), pwd.length(), kCompanyShortName, strlen(kCompanyShortName));
    
    FileUtils::getInstance()->loadFileMappingFromFile("repository/repo.mapping");
    
	// repo.mapping in xxtea encode
	FileUtils::getInstance()->loadFileMappingFromFile("res/repository/repo.mapping");
	// 手动存放obb目录时有效，走更新流程时与shenhe方式无异 by huangwei 19/11/20
	FileUtils::getInstance()->loadFileMappingFromFile("res/repository/obb.repo.mapping");

	// 为支持新的diff更新机制
	// lua代码不再以zip包方式发布，而是单文件lz压缩后xxtea加密，参考tjDecompress
	// 包括一些明文资源，如atlas、json（plist不再其列）

	// 考虑64架构增多，减少维护量级，抓取lua异常到行级
	// 放宽反破解强度，代码不再区分32位和64位

//#if CC_64BITS
#ifdef _NO_USED_
	engine->getLuaStack()->addSearchPath("cocos_x64");
	engine->getLuaStack()->addSearchPath("updater_x64");
#else
	engine->getLuaStack()->addSearchPath("cocos");
	engine->getLuaStack()->addSearchPath("updater");
#endif

	// lua文件名格式判断，可能是有目录的未加密源文件，也可能是没有目录的加密文件
	std::string testPath = FileUtils::getInstance()->fullPathForFilename("updater/main.lua");
	if (testPath.empty())
	{
		testPath = FileUtils::getInstance()->fullPathForFilename("updater/main");
		if (!testPath.empty())
			LuaEngine::isFlatPath = true;
	}

	// load updater screen
	if (engine->executeString("require 'main'"))
	{
	}

}

void AppDelegate::updateSuccessCallBack()
{
	// remove old lua engine
	ScriptEngineManager::getInstance()->removeScriptEngine();

	// new lua engine and run
	auto engine = LuaEngine::getInstance();
	ScriptEngineManager::getInstance()->setScriptEngine(engine);
	register_spine41(engine->getLuaStack()->getLuaState());

	// rebuild search path
	auto utils = FileUtils::getInstance();
	utils->setSearchPaths(_initSearchPaths);
	assetsManager->setLocalSearchPath(true);

	// read the lastest patch. by huangwei 2019/12/10
	char buf[256];
	VersionPlistInfo versionPlist = getLocalVersion();
	sprintf(buf, "%d", versionPlist.patch);
	SetUserInfo("patch", buf);

//#if CC_64BITS
#ifdef _NO_USED_
	engine->getLuaStack()->addSearchPath("cocos_x64");
	engine->getLuaStack()->addSearchPath("src_x64");
#else
	engine->getLuaStack()->addSearchPath("cocos");
	engine->getLuaStack()->addSearchPath("src");
#endif

	// pop updater screen
	Director::getInstance()->popToSceneStackLevel(0);

	// run lua
	if (engine->executeString("require 'main'"))
	{
	}

	// release assets manager
	assetsManager->autorelease();
}
