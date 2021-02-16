using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Reflection;

namespace Storm_CsHelper.Source.UI
{
    public static class UIHelper
    {
        // It was really not easy to do... Heck C#... C++ templates are really powerful compared to this crap!
        public static void RaisePropertyChanged(this object caller, [System.Runtime.CompilerServices.CallerMemberName] string propertyName = "")
        {
            FieldInfo firstMb = caller.GetType().GetField(
                "PropertyChanged",
                BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Public | BindingFlags.FlattenHierarchy
            );

            object handlerRaw = firstMb.GetValue(caller);
            Delegate handler = handlerRaw as Delegate;

            handler?.DynamicInvoke(caller, new PropertyChangedEventArgs(propertyName));
        }
    }
}
