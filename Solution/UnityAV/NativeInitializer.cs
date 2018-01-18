using System;
using System.Collections;
using System.Runtime.InteropServices;
using UnityEngine;

namespace UnityAV
{
    /// <summary>
    /// Responsible for the initialization of common native plugin components 
    /// </summary>
    internal static class NativeInitializer
    {
        private delegate void LogDelegate(string message);

        [DllImport("UnityAV.Native")]
        private static extern IntPtr GetRenderEventFunc();

        [DllImport("UnityAV.Native")]
        private static extern void DeregisterAllCallbacks();

        [DllImport("UnityAV.Native")]
        private static extern void RegisterLogCallback(LogDelegate callback);

        [DllImport("UnityAV.Native")]
        private static extern void RegisterWarningCallback(LogDelegate callback);

        [DllImport("UnityAV.Native")]
        private static extern void RegisterErrorCallback(LogDelegate callback);

        private static bool Initialized = false;

        /// <summary>
        /// Initializes the native components, repeated calls will be safely ignored
        /// </summary>
        /// <param name="monoBehaviour">The MonoBehaviour to piggyback on for a few
        /// required calls</param>
        public static void Initialize(MonoBehaviour monoBehaviour)
        {
            if (!Initialized)
            {
                Initialized = true;

                if (monoBehaviour == null)
                {
                    Initialized = false;
                    throw new ArgumentNullException(nameof(monoBehaviour));
                }

                monoBehaviour.StartCoroutine(CallPluginAtEndOfFrames());
                RegisterLogCallback(DebugLogThunk);
                RegisterWarningCallback(WarningLogThunk);
                RegisterErrorCallback(ErrorMethodThunk);
            }
        }

        /// <summary>
        /// Tears down the native components, repeated calls will be safely ignored
        /// </summary>
        public static void Teardown()
        {
            if (Initialized)
            {
                DeregisterAllCallbacks();
            }
        }

        private static void DebugLogThunk(string message)
        {
            Debug.Log("UnityAV.Native: " + message);
        }

        private static void WarningLogThunk(string message)
        {
            Debug.LogWarning("UnityAV.Native: " + message);
        }

        private static void ErrorMethodThunk(string message)
        {
            Debug.LogError("UnityAV.Native: " + message);
        }

        private static IEnumerator CallPluginAtEndOfFrames()
        {
            while (true)
            {
                yield return new WaitForEndOfFrame();
                GL.IssuePluginEvent(GetRenderEventFunc(), 1);
            }
        }
    }
}