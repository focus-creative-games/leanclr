using System;
using Tests.Fixtures;

namespace Tests.Bugs
{
    internal class TC_GenericInterfaceOnEnableDispatch : TestCaseBase
    {
        interface IOnEnable<P1>
        {
            void OnEnable(P1 p1);
        }

        abstract class UIBaseContainer
        {
            public void SetActive<P1>(P1 p1)
            {
                (this as IOnEnable<P1>)?.OnEnable(p1);
            }
        }

        class UIToast : UIBaseContainer, IOnEnable<object[]>, IOnEnable<int>
        {
            public int OnEnableObjectArrayCount;
            public int OnEnableIntCount;

            public void OnEnable(object[] para)
            {
                OnEnableObjectArrayCount++;
            }

            public void OnEnable(int time)
            {
                OnEnableIntCount++;
                Assert.Fail("OnEnable(int) should not be called");
            }
        }

        class ToastHost
        {
            public UIToast ShowToast(params object[] para)
            {
                return OpenBox<UIToast, object[]>(para);
            }

            public T OpenBox<T, P1>(P1 p1) where T : UIBaseContainer, IOnEnable<P1>
            {
                T res = Activator.CreateInstance<T>();
                res.SetActive(p1);
                return res;
            }
        }

        [UnitTest]
        public void ShowToast_DispatchesToObjectArrayOnEnable()
        {
            var host = new UIToast();
            IOnEnable<object[]> onEnable = host;
            Assert.NotNull(onEnable);
            onEnable.OnEnable(new object[] { "toast" });

            Assert.Equal(1, host.OnEnableObjectArrayCount);
            Assert.Equal(0, host.OnEnableIntCount);
        }

        [UnitTest]
        public void SetActive_ObjectArray_DispatchesToObjectArrayOnEnable()
        {
            var toast = new UIToast();
            toast.SetActive(new object[] { "toast" });

            Assert.Equal(1, toast.OnEnableObjectArrayCount);
            Assert.Equal(0, toast.OnEnableIntCount);
        }

        [UnitTest]
        public void Cast_AsIOnEnableObjectArray_DispatchesToObjectArrayOnEnable()
        {
            var toast = new UIToast();
            var onEnable = toast as IOnEnable<object[]>;
            Assert.NotNull(onEnable);
            onEnable.OnEnable(new object[] { "toast" });

            Assert.Equal(1, toast.OnEnableObjectArrayCount);
            Assert.Equal(0, toast.OnEnableIntCount);
        }
    }
}
