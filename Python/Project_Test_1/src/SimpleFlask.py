from flask import Flask, request
import boto3

app = Flask(__name__)

# S3 bucket name (will be created by Terraform)
BUCKET = "my-customer-app-bucket-12345"
s3 = boto3.client("s3")

@app.route("/")
def home():
    return "Hello from Flask on AWS EC2!"

@app.route("/upload", methods=["POST"])
def upload():
    file = request.files["file"]
    s3.upload_fileobj(file, BUCKET, file.filename)
    return f"Uploaded {file.filename} to {BUCKET}"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)
