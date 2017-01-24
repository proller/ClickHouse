#pragma once
#include <future>
#include <memory>
#include <vector>
#include <zookeeper/zookeeper.h>
#include <Poco/Event.h>
#include <common/Common.h>


namespace zkutil
{
using ACLPtr = const ACL_vector *;
using Stat = Stat;

struct Op
{
public:
	Op() : data(new zoo_op_t)
	{
	}
	virtual ~Op()
	{
	}

	virtual std::string describe() = 0;

	std::unique_ptr<zoo_op_t> data;

	struct Remove;
	struct Create;
	struct SetData;
	struct Check;
};

struct Op::Remove : public Op
{
	Remove(const std::string & path_, int32_t version) : path(path_)
	{
		zoo_delete_op_init(data.get(), path.c_str(), version);
	}

	std::string describe() override
	{
		return "command: remove, path: " + path;
	}

private:
	std::string path;
};

struct Op::Create : public Op
{
	Create(const std::string & path_, const std::string & value_, ACLPtr acl, int32_t flags);

	std::string getPathCreated()
	{
		return created_path.data();
	}

	std::string describe() override
	{
		return "command: create"
			   ", path: "
			+ path + ", value: " + value;
	}

private:
	std::string path;
	std::string value;
	std::vector<char> created_path;
};

struct Op::SetData : public Op
{
	SetData(const std::string & path_, const std::string & value_, int32_t version) : path(path_), value(value_)
	{
		zoo_set_op_init(data.get(), path.c_str(), value.c_str(), value.size(), version, &stat);
	}

	std::string describe() override
	{
		return "command: set"
			   ", path: "
			+ path + ", value: " + value + ", version: " + std::to_string(data->set_op.version);
	}

private:
	std::string path;
	std::string value;
	Stat stat;
};

struct Op::Check : public Op
{
	Check(const std::string & path_, int32_t version) : path(path_)
	{
		zoo_check_op_init(data.get(), path.c_str(), version);
	}

	std::string describe() override
	{
		return "command: check, path: " + path;
	}

private:
	std::string path;
};

struct OpResult : public zoo_op_result_t
{
	/// Указатели в этой структуре указывают на поля в классе Op.
	/// Поэтому деструктор не нужен
};

using Ops = std::vector<std::unique_ptr<Op>>;
using OpResults = std::vector<OpResult>;
using OpResultsPtr = std::shared_ptr<OpResults>;
using Strings = std::vector<std::string>;

namespace CreateMode
{
	extern const int Persistent;
	extern const int Ephemeral;
	extern const int EphemeralSequential;
	extern const int PersistentSequential;
}

using EventPtr = std::shared_ptr<Poco::Event>;
}
