// Separate Spine 4.1 module. Existing sp.* Lua/native classes remain untouched.
#include "Spine41Lua.h"
#include <spine41/spine-cocos2dx.h>
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include <cmath>
#include <memory>
#include <string>
#include <typeinfo>

namespace {
const char *const kLuaType = "spine41.Node";
spine41::Cocos2dTextureLoader textureLoader;

class Spine41Node final : public spine41::SkeletonAnimation {
public:
    static Spine41Node *load(const std::string &skeletonPath, const std::string &atlasPath,
                             float scale, std::string &error) {
        auto files = cocos2d::FileUtils::getInstance();
        if (!files->isFileExist(skeletonPath) || !files->isFileExist(atlasPath)) {
            error = "Spine 4.1 skeleton or atlas file is missing";
            return nullptr;
        }
        std::unique_ptr<spine41::Atlas> atlas(new spine41::Atlas(atlasPath.c_str(), &textureLoader, true));
        if (atlas->getPages().size() == 0 || atlas->getRegions().size() == 0) {
            error = "Spine 4.1 atlas has no pages or regions";
            return nullptr;
        }
        for (size_t index = 0; index < atlas->getPages().size(); ++index) {
            if (!atlas->getPages()[index]->texture) {
                error = "Spine 4.1 atlas texture could not be loaded";
                return nullptr;
            }
        }
        std::unique_ptr<spine41::Cocos2dAtlasAttachmentLoader> loader(
            new spine41::Cocos2dAtlasAttachmentLoader(atlas.get()));
        spine41::SkeletonBinary reader(loader.get());
        reader.setScale(scale);
        spine41::SkeletonData *data = reader.readSkeletonDataFile(skeletonPath.c_str());
        if (!data) {
            error = reader.getError().isEmpty() ? "Spine 4.1 skeleton could not be parsed" : reader.getError().buffer();
            return nullptr;
        }
        auto node = new Spine41Node();
        node->_atlas = atlas.release();
        node->_attachmentLoader = loader.release();
        node->_ownsAtlas = true;
        node->initWithData(data, true);
        node->update(0);
        node->autorelease();
        return node;
    }
};

int failure(lua_State *state, const char *message) {
    lua_pushnil(state);
    lua_pushstring(state, message);
    return 2;
}

Spine41Node *self(lua_State *state) {
    if (!tolua_isusertype(state, 1, kLuaType, 0, nullptr)) {
        luaL_error(state, "expected spine41.Node");
        return nullptr;
    }
    auto node = static_cast<Spine41Node *>(tolua_tousertype(state, 1, nullptr));
    if (!node) luaL_error(state, "Spine 4.1 node was released");
    return node;
}

int create(lua_State *state) {
    const char *skeleton = luaL_checkstring(state, 1);
    const char *atlas = luaL_checkstring(state, 2);
    float scale = static_cast<float>(luaL_optnumber(state, 3, 1));
    if (!std::isfinite(scale) || scale <= 0) return failure(state, "Spine 4.1 scale must be positive");
    std::string error;
    auto node = Spine41Node::load(skeleton, atlas, scale, error);
    if (!node) return failure(state, error.c_str());
    toluafix_pushusertype_ccobject(state, node->_ID, &node->_luaID, node, kLuaType);
    return 1;
}

int setAnimation(lua_State *state) {
    auto node = self(state);
    int track = static_cast<int>(luaL_checkinteger(state, 2));
    const char *name = luaL_checkstring(state, 3);
    luaL_checktype(state, 4, LUA_TBOOLEAN);
    if (track < 0 || !node->getSkeleton()->getData()->findAnimation(name))
        return failure(state, "Spine 4.1 animation or track does not exist");
    node->setAnimation(track, name, lua_toboolean(state, 4) != 0);
    node->update(0);
    lua_settop(state, 1);
    return 1;
}

int addAnimation(lua_State *state) {
    auto node = self(state);
    int track = static_cast<int>(luaL_checkinteger(state, 2));
    const char *name = luaL_checkstring(state, 3);
    luaL_checktype(state, 4, LUA_TBOOLEAN);
    float delay = static_cast<float>(luaL_optnumber(state, 5, 0));
    if (track < 0 || !std::isfinite(delay) || !node->getSkeleton()->getData()->findAnimation(name))
        return failure(state, "Spine 4.1 animation or track does not exist");
    node->addAnimation(track, name, lua_toboolean(state, 4) != 0, delay);
    lua_settop(state, 1);
    return 1;
}

int setSkin(lua_State *state) {
    auto node = self(state);
    const char *name = luaL_checkstring(state, 2);
    if (!node->getSkeleton()->getData()->findSkin(name)) return failure(state, "Spine 4.1 skin does not exist");
    node->setSkin(name);
    node->setSlotsToSetupPose();
    node->update(0);
    lua_settop(state, 1);
    return 1;
}

int setTimeScale(lua_State *state) {
    auto node = self(state);
    float scale = static_cast<float>(luaL_checknumber(state, 2));
    if (!std::isfinite(scale) || scale < 0) return failure(state, "Spine 4.1 time scale must be nonnegative");
    node->setTimeScale(scale);
    lua_settop(state, 1);
    return 1;
}

int getAnimations(lua_State *state) {
    auto node = self(state);
    auto &animations = node->getSkeleton()->getData()->getAnimations();
    lua_createtable(state, static_cast<int>(animations.size()), 0);
    for (size_t index = 0; index < animations.size(); ++index) {
        lua_createtable(state, 0, 2);
        lua_pushstring(state, animations[index]->getName().buffer());
        lua_setfield(state, -2, "name");
        lua_pushnumber(state, animations[index]->getDuration());
        lua_setfield(state, -2, "duration");
        lua_rawseti(state, -2, static_cast<int>(index + 1));
    }
    return 1;
}

int getBounds(lua_State *state) {
    auto rect = self(state)->getBoundingBox();
    lua_createtable(state, 0, 4);
    lua_pushnumber(state, rect.origin.x); lua_setfield(state, -2, "x");
    lua_pushnumber(state, rect.origin.y); lua_setfield(state, -2, "y");
    lua_pushnumber(state, rect.size.width); lua_setfield(state, -2, "width");
    lua_pushnumber(state, rect.size.height); lua_setfield(state, -2, "height");
    return 1;
}
}

int register_spine41(lua_State *state) {
    const int originalTop = lua_gettop(state);
    tolua_open(state);
    tolua_beginmodule(state, nullptr);
    tolua_usertype(state, "spine41.Node");
    tolua_module(state, "spine41", 0);
    tolua_beginmodule(state, "spine41");
    tolua_function(state, "create", create);
    lua_pushliteral(state, "4.1"); lua_setfield(state, -2, "runtimeVersion");
    tolua_cclass(state, "Node", "spine41.Node", "cc.Node", nullptr);
    tolua_beginmodule(state, "Node");
    tolua_function(state, "setAnimation", setAnimation);
    tolua_function(state, "addAnimation", addAnimation);
    tolua_function(state, "setSkin", setSkin);
    tolua_function(state, "setTimeScale", setTimeScale);
    tolua_function(state, "getAnimations", getAnimations);
    tolua_function(state, "getBounds", getBounds);
    tolua_endmodule(state);
    tolua_endmodule(state);
    tolua_endmodule(state);
    lua_settop(state, originalTop);
    g_luaType[typeid(Spine41Node).name()] = "spine41.Node";
    g_typeCast["Spine41Node"] = "spine41.Node";
    return 0;
}
