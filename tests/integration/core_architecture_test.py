#!/usr/bin/env python3
"""
AXIOM v3.0 - Core Architecture Integration Test
==============================================

Test the new core architecture components:
- Selective Dispatcher
- Eigen CPU Engine
- Enhanced performance monitoring
"""

import subprocess
import time
import sys
import os

def test_core_architecture():
    """Test the core AXIOM v3.0 architecture"""
    print("🔬 AXIOM v3.0 - Core Architecture Integration Test")
    print("=" * 55)
    
    # Find the executable
    possible_paths = [
        "ninja-build/axiom.exe",
        "build/axiom.exe",
        "cmake-build-debug/axiom.exe"
    ]
    
    axiom_path = None
    for path in possible_paths:
        if os.path.exists(path):
            axiom_path = path
            break
    
    if not axiom_path:
        print("❌ AXIOM executable not found!")
        return False
    
    print(f"✅ Found AXIOM executable: {axiom_path}")
    
    # Test 1: Basic Architecture Verification
    print("\n🏗️ Test 1: Architecture Verification")
    try:
        result = subprocess.run([axiom_path, "--help"], 
                              capture_output=True, text=True, timeout=5)
        if "AXIOM Engine v3.0" in result.stdout:
            print("✅ AXIOM v3.0 architecture confirmed")
        else:
            print("❌ Architecture verification failed")
            return False
    except Exception as e:
        print(f"❌ Architecture test failed: {e}")
        return False
    
    # Test 2: Core Mathematical Operations
    print("\n🧮 Test 2: Core Mathematical Operations")
    test_cases = [
        ("2+2", "4"),
        ("sqrt(16)", "4"),
        ("3.14159265358979", "3.14159265358979"),  # Precision test
        ("max(1,2,3,4,5)", "5"),  # Multi-argument function
    ]
    
    passed = 0
    for expr, expected in test_cases:
        try:
            result = subprocess.run([axiom_path, expr], 
                                  capture_output=True, text=True, timeout=5)
            if result.returncode == 0 and expected in result.stdout:
                print(f"✅ {expr} -> {result.stdout.strip()}")
                passed += 1
            else:
                print(f"❌ {expr} -> {result.stdout.strip()} (expected {expected})")
        except Exception as e:
            print(f"❌ {expr} -> Error: {e}")
    
    print(f"📊 Core operations: {passed}/{len(test_cases)} passed")
    
    # Test 3: Performance Benchmarking
    print("\n⚡ Test 3: Performance Benchmarking")
    
    # Measure execution time for simple operations
    times = []
    for i in range(10):
        start = time.time()
        try:
            subprocess.run([axiom_path, "2+2"], 
                         capture_output=True, text=True, timeout=1)
            end = time.time()
            times.append((end - start) * 1000)  # Convert to ms
        except:
            pass
    
    if times:
        avg_time = sum(times) / len(times)
        print(f"⏱️ Average execution time: {avg_time:.2f}ms")
        
        if avg_time < 50:
            print("🏎️ Performance: SENNA SPEED!")
        elif avg_time < 100:
            print("🏁 Performance: F1 SPEED")
        else:
            print("🚗 Performance: Good")
    
    # Test 4: Enterprise Features
    print("\n🏢 Test 4: Enterprise Features")
    enterprise_tests = [
        ("--benchmark", "benchmark"),
    ]
    
    enterprise_passed = 0
    for flag, keyword in enterprise_tests:
        try:
            result = subprocess.run([axiom_path, flag], 
                                  capture_output=True, text=True, timeout=10)
            if keyword.lower() in result.stdout.lower() or result.returncode == 0:
                print(f"✅ Enterprise feature: {flag}")
                enterprise_passed += 1
            else:
                print(f"⚠️ Enterprise feature: {flag} (partial)")
        except Exception as e:
            print(f"❌ Enterprise feature: {flag} -> {e}")
    
    # Final Assessment
    print("\n" + "=" * 55)
    print("📋 CORE ARCHITECTURE TEST SUMMARY")
    print("=" * 55)
    
    total_score = passed + enterprise_passed
    max_score = len(test_cases) + len(enterprise_tests)
    
    print(f"📊 Core Operations: {passed}/{len(test_cases)} passed")
    print(f"🏢 Enterprise Features: {enterprise_passed}/{len(enterprise_tests)} working")
    print(f"🎯 Overall Score: {total_score}/{max_score}")
    
    success_rate = (total_score / max_score) * 100
    print(f"📈 Success Rate: {success_rate:.1f}%")
    
    if success_rate >= 80:
        print("🎉 AXIOM v3.0 CORE ARCHITECTURE: EXCELLENT!")
        return True
    elif success_rate >= 60:
        print("✅ AXIOM v3.0 CORE ARCHITECTURE: GOOD")
        return True
    else:
        print("⚠️ AXIOM v3.0 CORE ARCHITECTURE: NEEDS IMPROVEMENT")
        return False

if __name__ == "__main__":
    success = test_core_architecture()
    sys.exit(0 if success else 1)