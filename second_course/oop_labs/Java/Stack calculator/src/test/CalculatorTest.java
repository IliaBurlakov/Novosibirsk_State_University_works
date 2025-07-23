package test;

import calculator.Context;
import calculator.commands.*;
import calculator.exceptions.*;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class CalculatorTest {

    @Test
    public void testPush() throws InvalidCommandArgumentException, StackUnderflowException {
        Context context = new Context();
        Push push = new Push();
        String[] argsForPush = {"PUSH", "5"};
        push.execute(context, argsForPush);
        assertFalse(context.getStack().isEmpty(), "Stack in not empty after PUSH");
        assertEquals(5.0, context.getStack().peek(), 0.0001, "Peek element is 5.0");
        Pop pop = new Pop();
        String[] argsForPop = {"POP"};
        pop.execute(context, argsForPop);
        assertTrue(context.getStack().isEmpty(), "Stack is empty after POP");
    }

    @Test
    public void testPopEmpty() {
        Context context = new Context();
        Pop pop = new Pop();
        String[] args = {"POP"};
        StackUnderflowException e = assertThrows(StackUnderflowException.class, () -> pop.execute(context, args), "Should throw StackUnderflowException when POP is executed on an empty stack");
        assertTrue(e.getMessage().contains("POP"), "Exception message should contain 'POP'");
    }

    @Test
    public void testAdd() throws InvalidCommandArgumentException, StackUnderflowException {
        Context context = new Context();
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "2"});
        push.execute(context, new String[]{"PUSH", "3"});
        Add add = new Add();
        add.execute(context, new String[]{"+"});
        assertEquals(5.0, context.getStack().peek(), 0.0001, "Peek element is 5.0 after ADD");
    }

    @Test
    public void testAddUnderflow() {
        Context context = new Context();
        Add add = new Add();
        String[] args = {"+"};
        StackUnderflowException e = assertThrows(StackUnderflowException.class, () -> add.execute(context, args), "Should throw StackUnderflowException when stack is empty");
        assertTrue(e.getMessage().contains("ADD"), "Exception message should contain 'ADD'");
    }

    @Test
    public void testSub() throws InvalidCommandArgumentException, StackUnderflowException {
        Context context = new Context();
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "10"});
        push.execute(context, new String[]{"PUSH", "4"});
        Sub sub = new Sub();
        sub.execute(context, new String[]{"-"});
        assertEquals(6.0, context.getStack().peek(), 0.0001, "Peek element is 6.0 after SUB");
    }

    @Test
    public void testMul() throws InvalidCommandArgumentException, StackUnderflowException {
        Context context = new Context();
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "3"});
        push.execute(context, new String[]{"PUSH", "5"});
        Mul mul = new Mul();
        mul.execute(context, new String[]{"*"});
        assertEquals(15.0, context.getStack().peek(), 0.0001, "Peek element is 15.0 after MUL");
    }

    @Test
    public void testDiv() throws InvalidCommandArgumentException, StackUnderflowException, DivisionByZeroException {
        Context context = new Context();
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "8"});
        push.execute(context, new String[]{"PUSH", "2"});
        Div div = new Div();
        div.execute(context, new String[]{"/"});
        assertEquals(4.0, context.getStack().peek(), 0.0001, "Peek element is 4.0 after DIV");
    }

    @Test
    public void testDivByZero() {
        Context context = new Context();
        context.getStack().push(10.0);
        context.getStack().push(0.0);
        Div div = new Div();
        String[] args = {"/"};
        DivisionByZeroException e = assertThrows(DivisionByZeroException.class, () -> div.execute(context, args), "Should throw DivisionByZeroException");
        assertTrue(e.getMessage().contains("DIV"), "Exception message should contain 'DIV'");
    }

    @Test
    public void testSqrt() throws InvalidCommandArgumentException, StackUnderflowException, RootOfANegativeNumberException {
        Context context = new Context();
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "9"});
        Sqrt sqrt = new Sqrt();
        sqrt.execute(context, new String[]{"SQRT"});
        assertEquals(3.0, context.getStack().peek(), 0.0001, "Peek element is 3.0 after SQRT");
    }

    @Test
    public void testSqrtOfNegative() {
        Context context = new Context();
        context.getStack().push(-4.0);
        Sqrt sqrt = new Sqrt();
        String[] args = {"SQRT"};
        RootOfANegativeNumberException e = assertThrows(RootOfANegativeNumberException.class, () -> sqrt.execute(context, args), "Should throw RootOfANegativeNumberException when SQRT is executed on a negative number");
        assertTrue(e.getMessage().contains("SQRT"), "Exception message should contain 'SQRT'");
    }

    @Test
    public void testDefineAndPush() throws InvalidCommandArgumentException {
        Context context = new Context();
        Define define = new Define();
        define.execute(context, new String[]{"DEFINE", "a", "7"});
        Push push = new Push();
        push.execute(context, new String[]{"PUSH", "a"});
        assertFalse(context.getStack().isEmpty(), "Stack in not empty after PUSH with variable");
        assertEquals(7.0, context.getStack().peek(), 0.0001, "Peek element is 7.0 after pushing variable 'a'");
    }

    @Test
    public void testPushInvalidArgument() {
        Context context = new Context();
        Push push = new Push();
        String[] args = {"PUSH", "unknownVar"};
        InvalidCommandArgumentException e = assertThrows(
                InvalidCommandArgumentException.class,
                () -> push.execute(context, args),
                "Should throw InvalidCommandArgumentException for unknown variable"
        );
        assertTrue(e.getMessage().contains("PUSH"), "Exception message should contain 'PUSH'");
    }

    @Test
    public void testComplexExpression() throws InvalidCommandArgumentException, StackUnderflowException, DivisionByZeroException {
        Context context = new Context();
        Push push = new Push();
        Add add = new Add();
        Sub sub = new Sub();
        Mul mul = new Mul();
        Div div = new Div();

        push.execute(context, new String[]{"PUSH", "3.25"});
        push.execute(context, new String[]{"PUSH", "2.75"});
        add.execute(context, new String[]{"+"});
        push.execute(context, new String[]{"PUSH", "7.5"});
        push.execute(context, new String[]{"PUSH", "1"});
        sub.execute(context, new String[]{"-"});
        mul.execute(context, new String[]{"*"});
        push.execute(context, new String[]{"PUSH", "2.5"});
        div.execute(context, new String[]{"/"});

        assertEquals(15.6, context.getStack().peek(), 0.0001, "Complex floating point expression result is 15.6");
    }
}
