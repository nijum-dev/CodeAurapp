
import { useState } from "react";
import "./styles/App.css";

const starterCode = `public class Test {
    public static void main(String[] args) {
        int a = 10;
        int b = 20;
        int sum = a + b;

        System.out.println(sum);
    }
}`;

export default function App() {
    const [source, setSource] = useState(starterCode);
    const [result, setResult] = useState(null);
    const [loading, setLoading] = useState(false);

    async function compileCode() {
        setLoading(true);
        setResult(null);

        try {
            const response = await fetch("/api/compile", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({ source })
            });

            const data = await response.json();

            if (!response.ok) {
                throw new Error(data.errors?.join(", ") || `Server error: ${response.status}`);
            }

            setResult(data);
        } catch (error) {
            setResult({
                success: false,
                errors: [
                    error.message || "Could not connect to Java compiler.",
                    "Make sure the Java backend is running on port 8080."
                ]
            });
        } finally {
            setLoading(false);
        }
    }

    return (
        <main className="app">
            <header className="header">
                <div>
                    <h1>Java Compiler</h1>
                    <p>Compiler Construction Project</p>
                </div>
                <span className="language-tag">Java · JFlex · CUP</span>
            </header>

            <section className="editor-section">
                <div className="section-heading">
                    <div>
                        <h2>Java Source Code</h2>
                        <p className="hint">Write your Java program below.</p>
                    </div>

                    <button
                        className="run-button"
                        onClick={compileCode}
                        disabled={loading || !source.trim()}
                    >
                        {loading ? "Compiling..." : "▶ Compile Java"}
                    </button>
                </div>

                <textarea
                    className="code-editor"
                    spellCheck="false"
                    value={source}
                    onChange={(event) => setSource(event.target.value)}
                    placeholder="Write Java code here..."
                />
            </section>

            {result && (
                <section className="results">
                    <div className="result-status">
                        <h2>Compilation Results</h2>
                        <span className={result.success ? "success" : "failure"}>
                            {result.success
                                ? "✓ Compilation Successful"
                                : "✕ Compilation Failed"}
                        </span>
                    </div>

                    {result.errors?.length > 0 && (
                        <div className="panel error-panel">
                            <h3>Compiler Errors</h3>
                            {result.errors.map((error, index) => (
                                <p key={index}>{error}</p>
                            ))}
                        </div>
                    )}

                    {result.tokens && (
                        <div className="panel">
                            <h3>1. Lexical Analysis — Tokens</h3>
                            <div className="table-wrapper">
                                <table>
                                    <thead>
                                        <tr>
                                            <th>Token Type</th>
                                            <th>Lexeme</th>
                                            <th>Line</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        {result.tokens.map((token, index) => (
                                            <tr key={index}>
                                                <td>{token.type}</td>
                                                <td>{token.lexeme || "EOF"}</td>
                                                <td>{token.line}</td>
                                            </tr>
                                        ))}
                                    </tbody>
                                </table>
                            </div>
                        </div>
                    )}

                    {result.ast && (
                        <div className="panel">
                            <h3>2. Syntax Analysis — Abstract Syntax Tree</h3>
                            <pre>{JSON.stringify(result.ast, null, 2)}</pre>
                        </div>
                    )}

                    {result.symbols && (
                        <div className="panel">
                            <h3>3. Semantic Analysis — Symbol Table</h3>
                            {result.symbols.length > 0 ? (
                                <div className="table-wrapper">
                                    <table>
                                        <thead>
                                            <tr>
                                                <th>Variable</th>
                                                <th>Data Type</th>
                                                <th>Line</th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            {result.symbols.map((symbol, index) => (
                                                <tr key={index}>
                                                    <td>{symbol.name}</td>
                                                    <td>{symbol.type}</td>
                                                    <td>{symbol.line}</td>
                                                </tr>
                                            ))}
                                        </tbody>
                                    </table>
                                </div>
                            ) : (
                                <p>No variables declared.</p>
                            )}
                        </div>
                    )}

                    {result.tac && (
                        <div className="panel">
                            <h3>4. Intermediate Code — Three Address Code</h3>
                            <pre>
                                {result.tac.length > 0
                                    ? result.tac.join("\n")
                                    : "No intermediate code generated."}
                            </pre>
                        </div>
                    )}
                </section>
            )}

            <footer className="footer">
                Java Compiler · Compiler Construction
            </footer>
        </main>
    );
}
