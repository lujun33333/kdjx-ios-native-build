//
//  SDKDelegate.m
//  MyLuaGame
//
//  Created by MacMini2 on 17/8/23.
//
//

#import "SDKDelegate.h"
#import <Security/Security.h>

#import "cocos2d.h"
#import "CCLuaEngine.h"
#import "CCLuaBridge.h"

#include <mutex>
#include <string>
#include <unordered_set>
using namespace cocos2d;

static std::mutex sdkreqs_mutex;
static std::unordered_set <SDKImpl*> sdkreqs;

//////////////////// CONFUSE_CODE BEGIN
void print_result()
{
    std::string S = "hello,word";
    std::string res = "hello,word";
    for (int i = (int)S.size() - 1; i >= 0; --i) {
        if (S[i]) res.push_back(S[i]);
    }
    for (int i = 0; i < S.size(); ++i) {
        if (S[i]==',') continue;
        res.insert(res.begin() + i, S[i]);
    }
}
//////////////////// CONFUSE_CODE END

@implementation SDKDelegate

+(void) proxy: (NSDictionary *)dic
{
    NSString *bundle = [dic objectForKey:@"bundle"];
    NSString *callback = [dic objectForKey:@"callback"];
    NSString *fname = [dic objectForKey:@"funcName"];

    int handlerID = (int)[callback integerValue];
    SDKImpl* sdk = new SDKImpl([fname UTF8String], [bundle UTF8String], handlerID);
    {
        std::lock_guard<std::mutex> lk(sdkreqs_mutex);
        sdkreqs.insert(sdk);
    }

    //////////////////// CONFUSE_CODE_CALL BEGIN
    print_result();
    //////////////////// CONFUSE_CODE_CALL END

    if ([fname isEqual: @"login"]) {
        // 登陆
        sdk->login();

    } else if ([fname isEqual:@"commitRoleInfo"]) {
        // 提交用户数据
        sdk->commit();

    } else if ([fname isEqual:@"pay"]) {
        // 支付
        sdk->pay();

    } else if ([fname isEqual:@"logout"]) {
        //logout登出
        sdk->logout();

    } else if ([fname isEqual:@"isHasNotchScreen"]) {
        // 刘海屏判断
        if (@available(iOS 11.0, *)) {
            CGFloat safeBottom = [[UIApplication sharedApplication] delegate].window.safeAreaInsets.bottom;
            if (safeBottom > 0) {
                [self proxyCallback:sdk withData:@"1"];
            }else{
                [self proxyCallback:sdk withData:@"0"];
            }
        } else {
            [self proxyCallback:sdk withData:@"0"];
        }
	} else {
		NSLog(@"no impl %@", fname);
	}
}

+(void) proxyCallbackDelete: (SDKImpl *)sdk
{
    {
        std::lock_guard<std::mutex> lk(sdkreqs_mutex);
        sdkreqs.erase(sdk);
    }
    delete sdk;
}

+(std::vector<SDKImpl*>) getProxyCallbackByName: (NSString *)name;
{

    std::string fname = [name UTF8String];
    std::vector<SDKImpl*> ret;

    {
        std::lock_guard<std::mutex> lk(sdkreqs_mutex);
        for (auto it = sdkreqs.begin(); it != sdkreqs.end(); ++it)
        {
            SDKImpl* p = *it;
            if (p->funcName == fname)
                ret.push_back(p);
        }
    }

    return ret;
}

+(void) proxyCallback: (SDKImpl *) sdk withData:(NSString *)data
{

	bool isExisted = true;
	{
        std::lock_guard<std::mutex> lk(sdkreqs_mutex);
        isExisted = sdkreqs.find(sdk) != sdkreqs.end();
    }
	// 不存在，可能已经被释放
	// 且意味着可能有其他请求残留
	// 比如游戏层未正确拦截多次支付
	if (!isExisted)
	{
		NSLog(@"%p was be deleted!!!", sdk);
		return;
	}

    int handlerID = sdk->handlerID;
    LuaStack *stack = LuaBridge::getStack();  //获取lua栈
    const char* cstr = [data UTF8String];

    LuaBridge::pushLuaFunctionById(handlerID); //压入需要调用的方法id（假设方法为XG）
    stack->pushString(cstr);  //将需要通过方法XG传递给lua的参数压入lua栈
    stack->executeFunction(1);  //根据压入的方法id调用方法XG，并把XG方法参数传递给lua代码
    LuaBridge::releaseLuaFunctionById(handlerID);

    {
        std::lock_guard<std::mutex> lk(sdkreqs_mutex);
        sdkreqs.erase(sdk);
    }
    delete sdk;
}

@end

NSDictionary* stdstring2NSDict(const std::string& jsonStr)
{
    NSString *s = [[NSString alloc] initWithUTF8String: jsonStr.c_str()];
    NSData *jsonData = [s dataUsingEncoding:NSUTF8StringEncoding];
    NSError *err = nil;
    NSDictionary *dic = [NSJSONSerialization JSONObjectWithData:jsonData
                                                        options:NSJSONReadingMutableContainers
                                                          error:&err];
    if (err)
    {
        NSLog(@"json error %@", [err localizedDescription]);
        return nil;
    }
    return dic;
}

NSString* NSDict2NSString(const NSDictionary* dic)
{
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dic options:0 error:0];
    NSString *jStr = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return jStr;
}

// Invoked directly by luaoc; credentials never enter SDK proxy logging.
@interface KDJXCredentialStore : NSObject
+ (NSString *)load:(NSDictionary *)arguments;
+ (NSNumber *)save:(NSDictionary *)arguments;
+ (NSNumber *)remove:(NSDictionary *)arguments;
@end

@implementation KDJXCredentialStore
+ (NSMutableDictionary *)query:(NSDictionary *)arguments
{
    id account = [arguments objectForKey:@"account"];
    if (![account isKindOfClass:[NSString class]] || [account length] == 0) {
        return nil;
    }
    NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleID.length == 0) {
        return nil;
    }
    return [NSMutableDictionary dictionaryWithDictionary:@{
        (id)kSecClass: (id)kSecClassGenericPassword,
        (id)kSecAttrService: [bundleID stringByAppendingString:@".private-login"],
        (id)kSecAttrAccount: account
    }];
}

+ (NSString *)load:(NSDictionary *)arguments
{
    NSMutableDictionary *query = [self query:arguments];
    if (!query) return @"";
    [query setObject:@YES forKey:(id)kSecReturnData];
    [query setObject:(id)kSecMatchLimitOne forKey:(id)kSecMatchLimit];
    CFTypeRef result = NULL;
    OSStatus status = SecItemCopyMatching((CFDictionaryRef)query, &result);
    if (status != errSecSuccess || !result) {
        if (result) CFRelease(result);
        return @"";
    }
    NSString *password = [[[NSString alloc] initWithData:(NSData *)result
                                              encoding:NSUTF8StringEncoding] autorelease];
    CFRelease(result);
    return password ?: @"";
}

+ (NSNumber *)save:(NSDictionary *)arguments
{
    NSMutableDictionary *query = [self query:arguments];
    id password = [arguments objectForKey:@"password"];
    if (!query || ![password isKindOfClass:[NSString class]] || [password length] == 0) {
        return @NO;
    }
    NSData *data = [password dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *attributes = @{
        (id)kSecValueData: data,
        (id)kSecAttrAccessible: (id)kSecAttrAccessibleWhenUnlockedThisDeviceOnly
    };
    OSStatus status = SecItemUpdate((CFDictionaryRef)query, (CFDictionaryRef)attributes);
    if (status == errSecItemNotFound) {
        [query addEntriesFromDictionary:attributes];
        status = SecItemAdd((CFDictionaryRef)query, NULL);
    }
    return @(status == errSecSuccess);
}

+ (NSNumber *)remove:(NSDictionary *)arguments
{
    NSMutableDictionary *query = [self query:arguments];
    if (!query) return @NO;
    OSStatus status = SecItemDelete((CFDictionaryRef)query);
    return @(status == errSecSuccess || status == errSecItemNotFound);
}
@end
