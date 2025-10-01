import boto3
import sys
import os

def upload_file(bucket, local_path, key=None, region="us-east-1"):
    s3 = boto3.client("s3", region_name=region)
    if key is None:
        key = os.path.basename(local_path)
    s3.upload_file(local_path, bucket, key)
    print(f"Uploaded {local_path} -> s3://{bucket}/{key}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python upload_csv.py <bucket> <local_csv_path> [s3_key]")
        sys.exit(1)
    bucket = sys.argv[1]
    local = sys.argv[2]
    key = sys.argv[3] if len(sys.argv) > 3 else None
    upload_file(bucket, local, key)
