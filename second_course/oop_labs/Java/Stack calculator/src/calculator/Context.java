package calculator;

import java.util.Stack;
import java.util.HashMap;
import java.util.Map;

public class Context {
    private Stack<Double> stack = new Stack<>();
    private Map<String, Double> definitions = new HashMap<>();

    public Stack<Double> getStack() {
        return stack;
    }

    public Map<String, Double> getDefinitions() {
        return definitions;
    }

    @Override
    public String toString() {
        return "Stack: " + stack.toString() + ", Definitions: " + definitions.toString();
    }
}