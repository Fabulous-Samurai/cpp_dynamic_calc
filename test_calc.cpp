#include <iostream>
#include <cassert>
#include <cmath>
#include "dynamic_calc.h"

void test_basic_operations() {
    Dynamic_calc calc;
    
    // Test addition
    auto result = calc.Evaluate("3 + 5");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 8.0) < 1e-9);
    std::cout << "✓ Addition test passed\n";
    
    // Test precedence
    result = calc.Evaluate("3 + 5 * 2");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 13.0) < 1e-9);
    std::cout << "✓ Precedence test passed\n";
    
    // Test power
    result = calc.Evaluate("2 ^ 3");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 8.0) < 1e-9);
    std::cout << "✓ Power test passed\n";
}

void test_unary_operations() {
    Dynamic_calc calc;
    
    // Test sqrt
    auto result = calc.Evaluate("sqrt 16");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 4.0) < 1e-9);
    std::cout << "✓ Sqrt test passed\n";
    
    // Test sin
    result = calc.Evaluate("sin 90");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 1.0) < 1e-9);
    std::cout << "✓ Sin test passed\n";
    
    // Test arcsec (valid input >= 1)
    result = calc.Evaluate("arcsec 2");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 60.0) < 1e-9);
    std::cout << "✓ Arcsec test passed\n";
    
    // Test arccsc (valid input >= 1)
    result = calc.Evaluate("arccsc 2");
    assert(result.err == CalcErr::None);
    assert(std::abs(result.result.value() - 30.0) < 1e-9);
    std::cout << "✓ Arccsc test passed\n";
}

void test_error_handling() {
    Dynamic_calc calc;
    
    // Test divide by zero
    auto result = calc.Evaluate("5 / 0");
    assert(result.err == CalcErr::DivideByZero);
    std::cout << "✓ Divide by zero error test passed\n";
    
    // Test 0/0 indeterminate
    result = calc.Evaluate("0 / 0");
    assert(result.err == CalcErr::IndeterminateResult);
    std::cout << "✓ Indeterminate result error test passed\n";
    
    // Test negative root
    result = calc.Evaluate("sqrt ( 0 - 1 )");
    assert(result.err == CalcErr::NegativeRoot);
    std::cout << "✓ Negative root error test passed\n";
    
    // Test domain error for arcsec (input < 1)
    result = calc.Evaluate("arcsec 0.5");
    assert(result.err == CalcErr::DomainError);
    std::cout << "✓ Arcsec domain error test passed\n";
    
    // Test domain error for arccsc (input < 1)
    result = calc.Evaluate("arccsc 0.5");
    assert(result.err == CalcErr::DomainError);
    std::cout << "✓ Arccsc domain error test passed\n";
}

int main() {
    std::cout << "Running calculator tests...\n\n";
    
    test_basic_operations();
    std::cout << "\n";
    
    test_unary_operations();
    std::cout << "\n";
    
    test_error_handling();
    std::cout << "\n";
    
    std::cout << "All tests passed! ✓\n";
    return 0;
}
