using System.Collections.Generic;
using System.Xml.Linq;

namespace Storm_CsHelper.Source.Xml
{
    static class XmlHelper
    {
        #region Delegates

        public delegate void HandleSetterDelegate(string value);
        public delegate void HandleAdderDelegate(string value);
        public delegate void HandleLoadXMLDelegate(XElement value);

        #endregion

        #region Methods

        static public string removeUnsupportedXMLTagCharacter(string futureTagStr)
        {
            return futureTagStr
                .Replace('\t', '_')
                .Replace('\n', '_')
                .Replace(' ', '_')
                .Replace('!', '_')
                .Replace('"', '_')
                .Replace('\'', '_')
                .Replace('#', '_')
                .Replace('$', '_')
                .Replace('%', '_')
                .Replace('&', '_')
                .Replace('(', '_')
                .Replace(')', '_')
                .Replace('*', '_')
                .Replace('+', '_')
                .Replace(',', '_')
                .Replace('/', '_')
                .Replace(':', '_')
                .Replace(';', '_')
                .Replace('<', '_')
                .Replace('=', '_')
                .Replace('>', '_')
                .Replace('?', '_')
                .Replace('@', '_')
                .Replace('[', '_')
                .Replace('\\', '_')
                .Replace(']', '_')
                .Replace('^', '_')
                .Replace('`', '_')
                .Replace('~', '_')
                .Replace('}', '_')
                .Replace('|', '_')
                .Replace('{', '_')
                ;
        }

        static public void retrieveXMLAttributeIfExists(XElement nodeToLoad, HandleSetterDelegate setter)
        {
            if (nodeToLoad != null)
            {
                setter(nodeToLoad.Value);
            }
        }

        static public void retrieveXMLAttributeIfExists(IEnumerable<XElement> nodeToLoad, HandleAdderDelegate adder)
        {
            if (nodeToLoad != null)
            {
                foreach (var node in nodeToLoad)
                {
                    if (node != null)
                    {
                        adder(node.Value);
                    }
                }
            }
        }

        static public void retrieveXMLAttributeIfExists(XAttribute attributeToLoad, HandleSetterDelegate setter)
        {
            if (attributeToLoad != null)
            {
                setter(attributeToLoad.Value);
            }
        }

        static public void retrieveXMLElementAndThrowIfNotExists(XElement root, string xmlValue, HandleSetterDelegate setter)
        {
            if (root != null)
            {
                var nodeToLoad = root.Element(xmlValue);
                if (nodeToLoad != null)
                {
                    setter(nodeToLoad.Value);
                }
                else
                {
                    throw new System.Exception(xmlValue + " not found in " + root.Name);
                }
            }
        }

        static public void retrieveXMLAttributeAndThrowIfNotExists(XElement root, string xmlValue, HandleSetterDelegate setter)
        {
            if (root != null)
            {
                var nodeToLoad = root.Attribute(xmlValue);
                if (nodeToLoad != null)
                {
                    setter(nodeToLoad.Value);
                }
                else
                {
                    throw new System.Exception(xmlValue + " not found in " + root.Name);
                }
            }
        }

        static public void LoadElementXMLFrom(XElement root, string xmlValue, HandleLoadXMLDelegate loader)
        {
            if (root != null)
            {
                var nodeToLoad = root.Elements(xmlValue);
                foreach (var node in nodeToLoad)
                {
                    if (node != null)
                    {
                        loader(node);
                    }
                }
            }
        }

        static public void LoadElementsXMLFrom(XElement root, string xmlValue, HandleLoadXMLDelegate loader)
        {
            if (root != null)
            {
                var nodeToLoad = root.Elements(xmlValue);
                foreach (var node in nodeToLoad)
                {
                    if (node != null)
                    {
                        loader(node);
                    }
                }
            }
        }

        static public void LoadAnyElementsXMLFrom(XElement root, HandleLoadXMLDelegate loader)
        {
            if (root != null)
            {
                var nodeToLoad = root.Elements();
                foreach (var node in nodeToLoad)
                {
                    if (node != null)
                    {
                        loader(node);
                    }
                }
            }
        }

        static public XElement AddSubElement(this XElement root, XName subElementName, object value)
        {
            root.Add(new XElement(subElementName, value));
            return root;
        }

        static public XElement AddSubElementAndDiveIntoIt(this XElement root, XName subElementName, object value)
        {
            XElement toAdd = new XElement(subElementName, value);
            root.Add(toAdd);
            return toAdd;
        }

        static public XElement LoadSubElementIfExist(this XElement root, XName name, HandleSetterDelegate setter)
        {
            retrieveXMLAttributeIfExists(root.Element(name), setter);
            return root;
        }

        static public XElement LoadSubElementIfExist(this XElement root, XName name, HandleLoadXMLDelegate loader)
        {
            //LoadElementXMLFrom(root, name.LocalName, loader);
            LoadElementsXMLFrom(root, name.LocalName, loader);
            return root;
        }

        static public XElement LoadSubElementArrayIfExist(this XElement root, XName name, HandleAdderDelegate adder)
        {
            retrieveXMLAttributeIfExists(root.Elements(name), adder);
            return root;
        }

        static public XElement LoadSubElementArrayIfExist(this XElement root, XName name, HandleLoadXMLDelegate loader)
        {
            LoadElementsXMLFrom(root, name.LocalName, loader);
            return root;
        }

        static public XElement LoadSubElementAndThrowIfNotExist(this XElement root, XName name, HandleSetterDelegate setter)
        {
            retrieveXMLElementAndThrowIfNotExists(root, name.LocalName, setter);
            return root;
        }

        static public XElement LoadAttributeIfExist(this XElement root, XName name, HandleSetterDelegate setter)
        {
            retrieveXMLAttributeIfExists(root.Attribute(name), setter);
            return root;
        }

        static public XElement LoadAttributeAndThrowIfNotExist(this XElement root, XName name, HandleSetterDelegate setter)
        {
            retrieveXMLAttributeAndThrowIfNotExists(root, name.LocalName, setter);
            return root;
        }

        #endregion
    }
}
