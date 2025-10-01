import os
import pandas as pd
import argparse
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
import joblib

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--train", type=str, default="/opt/ml/input/data/train/")
    parser.add_argument("--model-dir", type=str, default="/opt/ml/model")
    args = parser.parse_args()

    files = [os.path.join(args.train, f) for f in os.listdir(args.train) if f.endswith(".csv")]
    df = pd.read_csv(files[0])

    X = df.iloc[:, :-1]
    y = df.iloc[:, -1]

    model = RandomForestClassifier(n_estimators=100)
    model.fit(X, y)

    os.makedirs(args.model_dir, exist_ok=True)
    joblib.dump(model, os.path.join(args.model_dir, "model.joblib"))
    print("Model saved to", args.model_dir)
