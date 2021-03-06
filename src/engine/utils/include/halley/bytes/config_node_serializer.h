#pragma once

#include "halley/file_formats/config_file.h"
#include "halley/data_structures/maybe.h"
#include "halley/maths/colour.h"
#include "halley/maths/rect.h"
#include "config_node_serializer_base.h"
#include <set>

#include "halley/core/resources/resource_reference.h"

namespace Halley {
	template <>
	class ConfigNodeSerializer<bool> {
	public:
		ConfigNode serialize(bool value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}
		
		int deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
		{
			return node.asBool(false);
		}
	};

	template <>
    class ConfigNodeSerializer<int> {
    public:
        ConfigNode serialize(int value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}
		
		int deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return node.asInt(0);
        }
    };

	template <>
    class ConfigNodeSerializer<float> {
    public:
        ConfigNode serialize(float value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}
		
		float deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return node.asFloat(0);
        }
    };

	template <>
    class ConfigNodeSerializer<Vector2i> {
    public:
        ConfigNode serialize(Vector2i value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}
		
		Vector2i deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return node.asVector2i(Vector2i());
        }
    };

	template <>
    class ConfigNodeSerializer<Vector2f> {
    public:
        ConfigNode serialize(Vector2f value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}
		
		Vector2f deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return node.asVector2f(Vector2f());
        }
    };

	template <>
    class ConfigNodeSerializer<Angle1f> {
    public:
        ConfigNode serialize(Angle1f value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value.toRadians());
		}
		
		Angle1f deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return Angle1f::fromRadians(node.asFloat(0.0f));
        }
    };

	template <>
    class ConfigNodeSerializer<Colour4f> {
    public:
        ConfigNode serialize(Colour4f value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value.toString());
		}
		
		Colour4f deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			return Colour4f::fromString(node.asString("#000000"));
        }
    };

	template <>
    class ConfigNodeSerializer<Rect4i> {
    public:
        ConfigNode serialize(Rect4i value, ConfigNodeSerializationContext& context)
		{
        	std::vector<int> seq = { value.getX(), value.getY(), value.getWidth(), value.getHeight() };
        	return ConfigNode(seq);
		}
		
		Rect4i deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
        	if (node.getType() == ConfigNodeType::Sequence) {
				auto seq = node.asSequence();
				return Rect4i(seq[0].asInt(), seq[1].asInt(), seq[2].asInt(), seq[3].asInt());
			} else {
				return Rect4i();
			}
        }
    };

	template <>
    class ConfigNodeSerializer<Rect4f> {
    public:
        ConfigNode serialize(Rect4f value, ConfigNodeSerializationContext& context)
		{
        	std::vector<float> seq = { value.getX(), value.getY(), value.getWidth(), value.getHeight() };
        	return ConfigNode(seq);
		}
		
		Rect4f deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
        {
			if (node.getType() == ConfigNodeType::Sequence) {
				auto seq = node.asSequence();
				return Rect4f(seq[0].asFloat(0), seq[1].asFloat(0), seq[2].asFloat(0), seq[3].asFloat(0));
			} else {
				return Rect4f();
			}
        }
    };

	template <typename T>
    class ConfigNodeSerializer<std::optional<T>> {
    public:
        ConfigNode serialize(const std::optional<T>& value, ConfigNodeSerializationContext& context)
		{
        	if (value) {
				return ConfigNodeSerializer<T>::serialize(value.value(), context);
			} else {
				return ConfigNode();
			}
		}
		
        std::optional<T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
        {
        	if (node.getType() == ConfigNodeType::Undefined) {
				return std::optional<T>();
        	} else {
				return std::optional<T>(ConfigNodeSerializer<T>().deserialize(context, node));
			}
        }
    };

	template <typename T>
    class ConfigNodeSerializer<std::vector<T>> {
    public:
        ConfigNode serialize(const std::vector<T>& values, ConfigNodeSerializationContext& context)
		{
        	auto serializer = ConfigNodeSerializer<T>();
        	ConfigNode result = ConfigNode::SequenceType();
            auto& seq = result.asSequence();
        	seq.reserve(values.size());
        	for (auto& value: values) {
        		seq.push_back(serializer.serialize(value, context));
        	}
        	return result;
		}
		
        std::vector<T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
        {
			std::vector<T> result;
        	if (node.getType() == ConfigNodeType::Sequence) {
				auto seq = node.asSequence();
				result.reserve(seq.size());
        		for (auto& s: seq) {
					result.push_back(ConfigNodeSerializer<T>().deserialize(context, s));
				}
			}
			return result;
        }
	};

	template <typename T>
	class ConfigNodeSerializer<std::set<T>> {
	public:
		ConfigNode serialize(const std::set<T>& values, ConfigNodeSerializationContext& context)
		{
        	auto serializer = ConfigNodeSerializer<T>();
        	ConfigNode result = ConfigNode::SequenceType();
            auto& seq = result.asSequence();
        	seq.reserve(values.size());
        	for (auto& value: values) {
        		seq.push_back(serializer.serialize(value, context));
        	}
        	return result;
		}
		
        std::set<T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			std::set<T> result;
			if (node.getType() == ConfigNodeType::Sequence) {
				auto seq = node.asSequence();
				for (auto& s : seq) {
					result.insert(ConfigNodeSerializer<T>().deserialize(context, s));
				}
			}
			return result;
		}
	};

	template <typename T>
	class ConfigNodeSerializer<ResourceReference<T>> {
	public:
		ConfigNode serialize(const ResourceReference<T>& value, ConfigNodeSerializationContext& context)
		{
        	ConfigNode result = ConfigNode::MapType();
			result["asset"] = value.getId();
			return result;
		}
		
        ResourceReference<T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			const auto assetId = node["asset"].asString("");
			return ResourceReference<T>(assetId.isEmpty() ? std::shared_ptr<const T>() : context.resources->get<T>(assetId));
		}
	};
	
	template<>
	class ConfigNodeSerializer<String> {
	public:
		ConfigNode serialize(const String& value, ConfigNodeSerializationContext& context)
		{
			return ConfigNode(value);
		}

		String deserialize(ConfigNodeSerializationContext&, const ConfigNode& node)
		{
			return node.asString("");
		}
	};

	template <typename T>
	class ConfigNodeSerializer<std::map<String, T>>
	{
	public:
		ConfigNode serialize(const std::map<String, T>& values, ConfigNodeSerializationContext& context)
		{
        	auto serializer = ConfigNodeSerializer<T>();
        	ConfigNode result = ConfigNode::MapType();
        	for (auto& [key, value]: values) {
        		result[key] = serializer.serialize(value, context);
        	}
        	return result;
		}
		
		std::map<String, T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			std::map<String, T> result;
			if (node.getType() == ConfigNodeType::Map) {
				auto map = node.asMap();
				for (auto& s : map) {
					result[s.first] = ConfigNodeSerializer<T>().deserialize(context, s.second);
				}
			}
			return result;
		}
	};

	template <typename T>
	class ConfigNodeSerializer<OptionalLite<T>> {
	public:
        ConfigNode serialize(const OptionalLite<T>& value, ConfigNodeSerializationContext& context)
		{
        	if (value) {
				return ConfigNodeSerializer<T>::serialize(value.value(), context);
			} else {
				return ConfigNode();
			}
		}

		OptionalLite<T> deserialize(ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			if (node.getType() == ConfigNodeType::Undefined) {
				return OptionalLite<T>();
			} else {
				return OptionalLite<T>(ConfigNodeSerializer<T>().deserialize(context, node));
			}
		}
	};

	template <typename T>
	class ConfigNodeHelper {
	public:
		static ConfigNode serialize(const T& value, ConfigNodeSerializationContext& context)
		{
			return ConfigNodeSerializer<T>().serialize(value, context);
		}
		
		static void deserialize(T& dst, ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			if (node.getType() != ConfigNodeType::Undefined) {
				dst = ConfigNodeSerializer<T>().deserialize(context, node);
			}
		}
	};

	template <typename T>
	class ConfigNodeHelper<std::optional<T>> {
	public:
		static ConfigNode serialize(const std::optional<T>& src, ConfigNodeSerializationContext& context)
		{
			if (src) {
				return ConfigNodeHelper<T>::serialize(src.value(), context);
			} else {
				return ConfigNode();
			}
		}

		static void deserialize(std::optional<T>& dst, ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			dst = ConfigNodeSerializer<std::optional<T>>().deserialize(context, node);
		}
	};

	template <typename T>
	class ConfigNodeHelper<OptionalLite<T>> {
	public:
		static ConfigNode serialize(const OptionalLite<T>& src, ConfigNodeSerializationContext& context)
		{
			if (src) {
				return ConfigNodeHelper<T>::serialize(src.value(), context);
			} else {
				return ConfigNode();
			}
		}
		
		static void deserialize(OptionalLite<T>& dst, ConfigNodeSerializationContext& context, const ConfigNode& node)
		{
			dst = ConfigNodeSerializer<OptionalLite<T>>().deserialize(context, node);
		}
	};

	template <typename T>
	T ConfigNodeSerializerEnumUtils<T>::parseEnum(const ConfigNode& node)
	{
		return fromString<T>(node.asString());
	}

	template <typename T>
	ConfigNode ConfigNodeSerializerEnumUtils<T>::fromEnum(T value)
	{
		return ConfigNode(toString(value));
	}
}

