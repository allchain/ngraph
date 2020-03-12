//*****************************************************************************
// Copyright 2017-2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include <memory>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ngraph/attribute_visitor.hpp"
#include "ngraph/check.hpp"
#include "ngraph/except.hpp"
#include "ngraph/node.hpp"
#include "ngraph/opsets/opset.hpp"
#include "ngraph/output_vector.hpp"
#include "ngraph/util.hpp"
#include "node_factory.hpp"

class DictAttributeDeserializer : public ngraph::AttributeVisitor
{
public:
    DictAttributeDeserializer(const py::dict& attributes)
        : m_attributes(attributes)
    {
    }

    void on_attribute(const std::string& name, std::string& value) override
    {
        if (m_attributes.contains(name))
        {
            value = m_attributes[name.c_str()].cast<std::string>();
        }
    }
    void on_attribute(const std::string& name, bool& value) override
    {
        if (m_attributes.contains(name))
        {
            value = m_attributes[name.c_str()].cast<bool>();
        }
    }
    void on_adapter(const std::string& name, ngraph::ValueAccessor<void>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            // if (auto a = as_type<AttributeAdapter<ngraph::element::Type>>(&adapter))
            // {
            //     static_cast<ngraph::element::Type&>(*a) =
            //     m_attributes[name.c_str()].cast<ngraph::element::Type>();
            // }
            // else if (auto a = as_type<AttributeAdapter<ngraph::PartialShape>>(&adapter))
            // {
            //     static_cast<ngraph::PartialShape&>(*a) =
            //     m_attributes[name.c_str()].cast<ngraph::PartialShape>();
            // }
            NGRAPH_CHECK(
                false, "No AttributeVisitor support for accessing attribute named: ", name);
        }
    }
    void on_adapter(const std::string& name, ngraph::ValueAccessor<std::string>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<std::string>());
        }
    }
    void on_adapter(const std::string& name, ngraph::ValueAccessor<std::int64_t>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<std::int64_t>());
        }
    }
    void on_adapter(const std::string& name, ngraph::ValueAccessor<double>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<double>());
        }
    }
    void on_adapter(const std::string& name,
                    ngraph::ValueAccessor<std::vector<int64_t>>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<std::vector<std::int64_t>>());
        }
    }
    void on_adapter(const std::string& name,
                    ngraph::ValueAccessor<std::vector<std::string>>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<std::vector<std::string>>());
        }
    }
    void on_adapter(const std::string& name,
                    ngraph::ValueAccessor<std::vector<float>>& adapter) override
    {
        if (m_attributes.contains(name))
        {
            adapter.set(m_attributes[name.c_str()].cast<std::vector<float>>());
        }
    }

protected:
    // template <typename T>
    // void set_adapter_value(const std::string& key, ngraph::ValueAccessor<T>& adapter)
    // {
    //     if (m_attributes.contains(key))
    //     {
    //         adapter.set(m_attributes[key.c_str()]).cast<T&>();
    //     }
    // }

    const py::dict& m_attributes;
};

class NodeFactory
{
public:
    NodeFactory() {}
    NodeFactory(const std::string& opset_name)
        : m_opset{get_opset(opset_name)}
    {
    }

    std::shared_ptr<ngraph::Node> create(const std::string op_type_name,
                                         const ngraph::NodeVector& arguments,
                                         const py::dict& attributes = py::dict())
    {
        const auto op_node = std::shared_ptr<ngraph::Node>(m_opset.create(op_type_name));

        NGRAPH_CHECK(op_node != nullptr, "Couldn't create operator: ", op_type_name);
        DictAttributeDeserializer visitor(attributes);

        op_node->set_arguments(arguments);
        op_node->visit_attributes(visitor);
        op_node->constructor_validate_and_infer_types();

        return op_node;
    }

private:
    const ngraph::OpSet& get_opset(const std::string& opset_name)
    {
        const std::string& opset_name_{ngraph::to_lower(opset_name)};
        if (opset_name_ == "opset0")
        {
            return ngraph::get_opset0();
        }
        else if (opset_name_ == "opset1")
        {
            return ngraph::get_opset1();
        }
        else if (opset_name_ == "opset2")
        {
            return ngraph::get_opset2();
        }
        else
        {
            throw ngraph::ngraph_error("Unsupported opset version requested.");
        }
    }

    const ngraph::OpSet& m_opset{ngraph::get_opset0()};
};

namespace py = pybind11;

void regclass_pyngraph_NodeFactory(py::module m)
{
    py::class_<NodeFactory> node_factory(m, "NodeFactory");
    node_factory.doc() = "NodeFactory creates nGraph nodes";

    node_factory.def(py::init());
    node_factory.def(py::init<std::string>());

    node_factory.def("create", &NodeFactory::create);

    node_factory.def("__repr__", [](const NodeFactory& self) { return "<NodeFactory>"; });
}
